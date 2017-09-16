***********
ReadMe
***********

.. sectnum::

.. contents:: Table of Contents

Overview
~~~~~~~~~~~~~~~~~~~~~~~~~
The VTK and ParaView projects have a long history of regression testing using the tools (ctest, dashboards, etc) provided by CMake. With Qt as the user interface toolkit in ParaView, QtTesting library has been created for automatic testing of UI components in ParaView application. This document is intended to describe the new framework in sufficient detail to allow outside contribution.

Requirements
~~~~~~~~~~~~~~~~~~~~~~~~~
- The framework must have an open source license consistent with the ParaView / VTK license. Rationale: to allow all ParaView users to contribute and run test cases.

- The framework should support easy test case creation to encourage user-contributed tests and bug reporting.

- The framework should allow for multiple forms of validation whether a test passes or fails, including-but-not-limited to image-based validation.

- The framework should support test cases that can be recorded as scripts. Rationale: a test case recorded as a script can apply complex logic for quantitative verification and validation of results. For example, a scripted test case could check the values of internal data structures during playback, instead of relying on image-based validation.

- The framework should support test cases that can be recorded as low-level "metafiles". Rationale: simplicity, reduce dependencies on any given script engine for testing.

- Where possible, test-cases should continue to play-back correctly despite modifications to the user interface.

- To reduce developer burden, the framework should be as unintrusive as possible. In particular, it should be possible to use the framework with existing code and interfaces built using Qt Designer, and the framework should not require that the developer use "special" widgets, multiple inheritance, or the like.

Design Challenges
~~~~~~~~~~~~~~~~~~~~~~~~~
Low-Level vs. High-Level Events
********************************************

In theory, all user interaction with a graphical user interface can be broken-down into a handful of low-level "events": mouse button press, mouse button release, mouse movement, key press, and key release. Recording the low-level events and their parameters as they happen for recording, then synthesizing the same events in order for playback, will put the application into the same state. In practice, this approach does not work well due to differences in user interface "skins", platform, hardware, and operating environment:

- Differences in window manager between platforms (or even on the same platform, due to user choices) may cause the sizes and arrangement of windows to vary between test case recording and playback.

- Due to user interface skinning or platform specific look-and-feel, the sizes of widgets may very, even if the top-level window is consistently sized. As an example, the width of a splitter bar may vary by a few pixels between two platforms. During playback of a test case where the user resizes a splitter, this could cause the mouse to "miss" the splitter, sending the subsequent mouse movement events into adjacent controls instead of to the splitter where they belong. Differences in font size will affect the size of a menu. Internationalization can radically change the size and layout of widgets.

- When recording typical user interaction with a file dialog, the test case would include some combination of scrollbar movement and double-clicks as the user navigated through their filesystem. Unfortunately, this recording would be heavily dependent on the contents of the filesystem - nearly any change in filesystem contents between recording and playback will cause the test to fail, since filenames would be in different locations relative to the mouse.

- A recording of user interaction with a spin button would fail during playback if the spin button is replaced with a slider, despite the fact that both represent an integer quantity.

For these and many other similar issues, it isn't enough that a test case record what the user did - a test case should record what the user intended. Thus, test case recording and playback becomes a case of mapping between low level user interface events and high-level events - rather than recording a button activation as a mouse press event followed by a mouse release event at such-and-such coordinates, a single high-level "button activation" event can be recorded, so that the correct button can be activated during playback, regardless of its screen coordinates. Similarly, all of the mouse clicks and scrolling within a file dialog can be replaced by a single "file selected" event that includes the file name as a parameter - during playback, the correct file can be selected regardless of any changes to the underlying filesystem. When recording interaction with a spin button, the series of mouse-clicks and keyboard input can be replaced with a single "set integer" event that includes the final value as a parameter - during playback, the "set integer" event can be interpreted by any widget that represents integers (spin buttons, sliders, dials, etc), not just the type of widget used to record the test.

Widget Naming
********************************************
While the notion of mapping between low-level and high-level events is straightforward, one problem remains - how to provide a serializable name for widgets so that high-level events can be directed to the correct widget during playback. It is typical to use pointers or references to access Qt widgets at runtime, but these are obviously inadequate as serializable identifiers. The main problem is with generating globally-unique names, while allowing for reasonable changes to the user interface without breaking test cases. As an example, a test recorded on one system may be played-back on another, with a different configuration of docked / floating toolbars. Several alternatives have been discussed:
- Flat - Assign a globally-unique name to every widget. Pros: test cases work no matter how the UI is rearranged. Cons: Developers must explicitly name everything, names quickly become unwieldy, QtDesigner limits widget names (e.g. can't contain slashes), and QtDesigner-generated intance variables will share the long, unwieldy names.

- The MS Way - Same as "Flat", only use 128-bit UUIDs as identifiers. Insert Homer Simpson-like shudder here ...

- Widget Hierarchy - Use the Qt widget hierarchy to generate unique names. Pros: developers can use straightforward names for widgets in Qt designer, and only have to explicitly name top-level objects like dialogs. Cons: rearranging the widget hierarchy breaks test cases, e.g. floating/docking windows.

- The K-3D Way - Use a hierarchical naming scheme, but make it orthogonal to the Qt widget hierachy. Pros: same as "Widget Hierarchy", plus floating and docking windows no longer break test cases. Cons: since this is a separate hierarchy, it has to be maintained at runtime, e.g. by explicitly registering widgets with some central manager, implying a highly-intrusive mechanism of custom widgets or the like.

After considering these options it was decided that the framework would use the "Widget Hierarchy" method, generating widget names by walking the Qt widget hierarchy, concatenating object names (separated by slashes) into a hierarchical "path" string. Although this method is especially brittle in the face of UI modifications, it requires the least developer effort and integrates well with custom user interfaces and interfaces created with Qt designer.

Design
~~~~~~~~~~~~~~~~~~~~~~~~~
Recording
********************************************
Test case recording centers around the pqWidgetEventTranslator abstract interface class. Derivatives of this class are responsible for converting low-level Qt events ("mouse move", "button down", "button up", etc) into the high-level events (e.g: "button activated") that can be serialized and played back. pqWidgetEventTranslator derivatives may be particular to a specific widget type, such as pqComboBoxEventTranslator, or may represent a class of related widgets, such as pqAbstractSliderEventTranslator, which can translate events for any widget that derives from QAbstractSlider, including QDial, QScrollBar, and QSlider. pqWidgetEventTranslator derivatives implement the translateEvent() method, where they determine whether they can handle a Qt event, and if they can, convert it into a high-level event which consists of two strings: a command, and the corresponding command arguments (which may be empty). The translator then emits the recordEvent() signal one-or-more times to pass the high-level event(s) to its container. It is intended that the test framework eventually include pqEventWidgetTranslator derivatives for all "stock" Qt widgets.

A collection of pqWidgetEventTranslator objects is maintained at runtime by an instance of pqEventTranslator. pqEventTranslator hooks itself into the Qt event loop as a top-level event filter, so it receives every Qt event that occurs at runtime for the entire application. pqEventTranslator passes the Qt event to each of its pqWidgetEventTranslator instances in-turn, until one of the instances signals that the event has been "consumed". When a pqWidgetEventTranslator emits a high-level event, the event is "caught" by the pqEventTranslator instance and combined with the serializable address of the widget to which the high-level event applies. The three strings (address, command, and arguments) of the complete high-level event are emitted from the pqEventTranslator as a Qt signal.

The high-level event emitted from pqEventTranslator may be caught by any observer with the correct Qt slot. It is up to the observer(s) to serialize the high-level event for later playback. At this time the framework includes two observers, pqEventObserverStdout and pqEventObserverXML, which serialize the high-level events to stdout and an XML file respectively. Developers can create their own observers to implement custom functionality, such as serializing events to a logfile, a Python script, etc.

Playback
********************************************
Test case playback is basically a mirror-image of recording. The pqEventSource provides an abstract interface for objects capable of providing a "stream" of high-level events. The pqXMLEventSource provides a concrete implementation of pqEventSource, and is capable of reading the XML files generated by pqEventObserverXML.

The pqEventPlayer class maintains a collection of pqWidgetEventPlayer objects that handle translation from high-level events to low-level events. An instance of pqEventDispatcher retrieves events from a pqEventSource, handing them off to an instance of pqEventPlayer for playback.

Derivatives of the pqWidgetEventPlayer abstract interface class are responsible for mapping high-level events back into low-level Qt events (or direct widget manipulation). Note that there is not a one-to-one correspondence between pqWidgetEventTranslator and pqWidgetEventPlayer classes - a single pqAbstractIntEventPlayer object is capable of handling events generated by both pqSpinBoxEventTranslator and pqAbstractSliderEventTranslator, because the two translators map dissimilar Qt events into a single, abstract "set_int" event.

pqEventPlayer maintains a collection of pqWidgetEventPlayer objects, and as with pqEventTranslator, the set of pqWidgetEventPlayer objects can be instantiated by the developer or by pqEventTranslator, or both. Each high-level event is encoded as three strings(address, command, and arguments), which are fed to pqEventPlayer::playEvent(). pqEventPlayer decodes the address string and uses it to lookup the corresponding widget. The high-level event and widget are then passed to each pqWidgetEventPlayer in-turn, until one of them signals that the event has been handled.

Extensions
********************************************
The test framework can be extended through the creation of custom components. To store high-level events to a custom format or device, the developer creates an observer class with a correctly-prototyped slot to receive events from pqEventTranslator. To play the data back, a corresponding pqEventSource derivative is created that can read the data.

Support for test case recording and playback of custom widgets is handled by creating custom derivatives of pqWidgetEventTranslator and pqWidgetEventPlayer, then adding instances of those custom derivatives to pqEventTranslator and pqEventPlayer, respectively. pqEventTranslator and pqEventPlayer also have methods that instantiate all of the "built in" translators and players that are part of the framework. Because translators and players are executed in-order until a given event has been handled, the developer can "override" builtin translators/players by adding their custom components first.

Outstanding Issues
~~~~~~~~~~~~~~~~~~~~~~~~~
A significant issue with the design of the framework is the problem of modality - in general, dispatchers cannot assume that a call to pqEventPlayer::playEvent() will return immediately, since the event playback may cause a modal dialog or a widget-generated context menu to be shown. In these cases playEvent() will not return until the dialog is closed or the context menu is hidden, yet the test framework must continue dispatching events or there will never be any simulated user input to close the dialog or choose an item from the menu! For this reason pqEventDispatcher relies on the QAbstractEventDispatcher::aboutToBlock() signal to force asynchronous dispatching of events. Other methods of handling event dispatching have been tried, e.g pqThreadedEventDispatcher, and we consider this to be an area for further development.

A second tricky problem has been the mapping of high-level events to low-level Qt events for playback. In a nutshell, receiving useful events for recording is easy, because the API of a user interface toolkit such as Qt is fundamentally there to enable a client application to receive information about user interaction. Playback is more difficult because the synthesis of simulated user input is a highly specialized use case that is generally not supported or encouraged by the public API. This can lead to complex code, and generally requires an understanding of the internal implementation details of Qt widgets: on one hand, playing-back a button activation is simple, because the QAbstractButton::click() method is part of the API, while playing-back a menu activation is complex, because there is no comparable method for menu items. In the latter case, the implementation must synthesize low-level Qt events to simulate the corresponding user interaction.

Developer's Guide
~~~~~~~~~~~~~~~~~~~~~~~~~
Download and Build with CMake
********************************
- `QtTesting <https://github.com/Kitware/QtTesting/>`_
- `CMake <http://www.cmake.org/download/>`_


Add the testing framework into your application
************************************************************

Checkout the application example in the "Examples" directory, and follow these steps to add the testing framework to your application:

- Create a variable myQtTestingUtiliy - can be added to your mainWindow class (You can have your own EventObserver and EventSource), and initialize this variable as following::

    this->TestUtility = new myQtTestingUtility(this);
    this->TestUtility->addEventObserver("xml", new myXMLEventObserver(this));
    this->TestUtility->addEventSource("xml", new myXMLEventSource(this));

- Link UI buttons to the myQtTestUtility "record" and "play" slots::

    QObject::connect(Ui.RecordButton, SIGNAL(clicked(bool)), this, SLOT(record()));
    QObject::connect(Ui.PlayBackButton, SIGNAL(clicked(bool)), this, SLOT(play()));