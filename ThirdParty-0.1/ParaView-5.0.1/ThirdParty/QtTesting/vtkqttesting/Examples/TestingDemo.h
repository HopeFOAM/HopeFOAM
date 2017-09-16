

#include <QMainWindow>

class pqTestUtility;

class TestingDemo : public QMainWindow
{
  Q_OBJECT
public:
  TestingDemo();
  ~TestingDemo();
protected slots:
  void record();
  void play();
  void popup();


private:
  Q_DISABLE_COPY(TestingDemo)

  pqTestUtility *TestUtility;
};
