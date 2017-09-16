/* -*- c -*- *****************************************************************
** Copyright (C) 2010 Sandia Corporation
** Under the terms of Contract DE-AC04-94AL85000 with Sandia Corporation,
** the U.S. Government retains certain rights in this software.
**
** This source code is released under the New BSD License.
**
** This test provides a simple means of timing the IceT compositing.  It can be
** used for quick measurements and simple scaling studies.
*****************************************************************************/

#include <IceTDevCommunication.h>
#include <IceTDevContext.h>
#include <IceTDevImage.h>
#include <IceTDevMatrix.h>
#include "test_util.h"
#include "test_codes.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>

#ifndef M_E
#define M_E         2.71828182845904523536028747135266250   /* e */
#endif

/* Structure used to capture the recursive division of space. */
struct region_divide_struct {
    int axis;           /* x = 0, y = 1, z = 2: the index to a vector array. */
    float cut;          /* Coordinate where cut occurs. */
    int my_side;        /* -1 on the negative side, 1 on the positive side. */
    int num_other_side; /* Number of partitions on other side. */
    struct region_divide_struct *next;
};

typedef struct region_divide_struct *region_divide;

#define NAME_SIZE 32
typedef struct {
    IceTInt num_proc;
    char strategy_name[NAME_SIZE];
    char si_strategy_name[NAME_SIZE];
    IceTInt num_tiles_x;
    IceTInt num_tiles_y;
    IceTInt screen_width;
    IceTInt screen_height;
    IceTFloat zoom;
    IceTBoolean transparent;
    IceTBoolean no_interlace;
    IceTBoolean no_collect;
    IceTBoolean dense_images;
    IceTInt max_image_split;
    IceTInt frame_number;
    IceTDouble render_time;
    IceTDouble buffer_read_time;
    IceTDouble buffer_write_time;
    IceTDouble compress_time;
    IceTDouble interlace_time;
    IceTDouble blend_time;
    IceTDouble draw_time;
    IceTDouble composite_time;
    IceTDouble collect_time;
    IceTInt64 bytes_sent;
    IceTDouble frame_time;
} timings_type;

static timings_type *g_timing_log;
static IceTSizeType g_timing_log_size;

/* Array for quick opacity lookups. */
#define OPACITY_LOOKUP_SIZE 4096
#define OPACITY_MAX_DT 4
#define OPACITY_COMPUTE_VALUE(dt) (1.0 - pow(M_E, -(dt)))
#define OPACITY_DT_2_INDEX(dt) \
    (  ((dt) < OPACITY_MAX_DT) \
     ? (int)((dt)*(OPACITY_LOOKUP_SIZE/OPACITY_MAX_DT)) \
     : OPACITY_LOOKUP_SIZE )
#define OPACITY_INDEX_2_DT(index) \
    ((index)*((double)OPACITY_MAX_DT/OPACITY_LOOKUP_SIZE))
static IceTDouble g_opacity_lookup[OPACITY_LOOKUP_SIZE+1];
#define QUICK_OPACITY(dt) (g_opacity_lookup[OPACITY_DT_2_INDEX(dt)])

static void init_opacity_lookup(void)
{
    IceTSizeType idx;

    for (idx = 0; idx < OPACITY_LOOKUP_SIZE+1; idx++) {
        IceTDouble distance_times_tau = OPACITY_INDEX_2_DT(idx);
        g_opacity_lookup[idx] = OPACITY_COMPUTE_VALUE(distance_times_tau);
    }
}

/* Used to signal the first render of a frame. */
static IceTBoolean g_first_render;

/* Program arguments. */
static IceTInt g_num_tiles_x;
static IceTInt g_num_tiles_y;
static IceTInt g_num_frames;
static IceTInt g_seed;
static IceTFloat g_zoom;
static IceTBoolean g_transparent;
static IceTBoolean g_colored_background;
static IceTBoolean g_no_interlace;
static IceTBoolean g_no_collect;
static IceTBoolean g_use_callback;
static IceTBoolean g_dense_images;
static IceTBoolean g_sync_render;
static IceTBoolean g_write_image;
static IceTEnum g_strategy;
static IceTEnum g_single_image_strategy;
static IceTBoolean g_do_magic_k_study;
static IceTInt g_max_magic_k;
static IceTBoolean g_do_image_split_study;
static IceTInt g_min_image_split;
static IceTBoolean g_do_scaling_study_factor_2;
static IceTBoolean g_do_scaling_study_factor_2_3;
static IceTInt g_num_scaling_study_random;

static float g_color[4];

static void usage(char *argv[])
{
    printstat("\nUSAGE: %s [testargs]\n", argv[0]);
    printstat("\nWhere  testargs are:\n");
    printstat("  -tilesx <num> Sets the number of tiles horizontal (default 1).\n");
    printstat("  -tilesy <num> Sets the number of tiles vertical (default 1).\n");
    printstat("  -frames <num> Sets the number of frames to render (default 2).\n");
    printstat("  -seed <num>   Use the given number as the random seed.\n");
    printstat("  -zoom <num>   Set the zoom factor for the camera (larger = more zoom).\n");
    printstat("  -transparent  Render transparent images.  (Uses 4 floats for colors.)\n");
    printstat("  -colored-background Use a color for the background and correct as necessary.\n");
    printstat("  -no-interlace Turn off the image interlacing optimization.\n");
    printstat("  -no-collect   Turn off image collection.\n");
    printstat("  -use-callback Do the drawing in an IceT callback.\n");
    printstat("  -sync-render  Synchronize rendering by adding a barrier to the draw callback.\n");
    printstat("  -dense-images Composite dense images by classifying no pixels as background.\n");
    printstat("  -write-image  Write an image on the first frame.\n");
    printstat("  -reduce       Use the reduce strategy (default).\n");
    printstat("  -vtree        Use the virtual trees strategy.\n");
    printstat("  -sequential   Use the sequential strategy.\n");
    printstat("  -bswap        Use the binary-swap single-image strategy.\n");
    printstat("  -bswapfold    Use the binary-swap with folding single-image strategy.\n");
    printstat("  -radixk       Use the radix-k single-image strategy.\n");
    printstat("  -radixkr      Use the radix-kr single-image strategy.\n");
    printstat("  -tree         Use the tree single-image strategy.\n");
    printstat("  -magic-k-study <num> Use the radix-k single-image strategy and repeat for\n"
           "                   multiple values of k, up to <num>, doubling each time.\n");
    printstat("  -max-image-split-study <num> Repeat the test for multiple maximum image\n"
           "                   splits starting at <num> and doubling each time.\n");
    printstat("  -scaling-study-factor-2 Perform a scaling study for all process counts\n"
              "                that are a factor of 2.\n");
    printstat("  -scaling-study-factor-2-3 Perform a scaling study that includes all\n"
              "                process counts that are a factor of 2 plus all process\n"
              "                counts that are a factor of 3 plus most process counts\n"
              "                that have factors of 2 and 3.\n");
    printstat("  -scaling-study-random <num> Picks a random number to bifurcate the\n"
              "                processes and runs the compositing on each of them. This\n"
              "                experiment is run <num> times. Run enough times this test\n"
              "                should give performance over scales at odd process counts.\n");
    printstat("  -h, -help     Print this help message.\n");
    printstat("\nFor general testing options, try -h or -help before test name.\n");
}

static void parse_arguments(int argc, char *argv[])
{
    int arg;

    g_num_tiles_x = 1;
    g_num_tiles_y = 1;
    g_num_frames = 2;
    g_seed = (IceTInt)time(NULL);
    g_zoom = (IceTFloat)1.0;
    g_transparent = ICET_FALSE;
    g_colored_background = ICET_FALSE;
    g_no_interlace = ICET_FALSE;
    g_no_collect = ICET_FALSE;
    g_use_callback = ICET_FALSE;
    g_dense_images = ICET_FALSE;
    g_sync_render = ICET_FALSE;
    g_write_image = ICET_FALSE;
    g_strategy = ICET_STRATEGY_REDUCE;
    g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_AUTOMATIC;
    g_do_magic_k_study = ICET_FALSE;
    g_max_magic_k = 0;
    g_do_image_split_study = ICET_FALSE;
    g_min_image_split = 0;
    g_do_scaling_study_factor_2 = ICET_FALSE;
    g_do_scaling_study_factor_2_3 = ICET_FALSE;
    g_num_scaling_study_random = 0;

    for (arg = 1; arg < argc; arg++) {
        if (strcmp(argv[arg], "-tilesx") == 0) {
            arg++;
            g_num_tiles_x = atoi(argv[arg]);
        } else if (strcmp(argv[arg], "-tilesy") == 0) {
            arg++;
            g_num_tiles_y = atoi(argv[arg]);
        } else if (strcmp(argv[arg], "-frames") == 0) {
            arg++;
            g_num_frames = atoi(argv[arg]);
        } else if (strcmp(argv[arg], "-seed") == 0) {
            arg++;
            g_seed = atoi(argv[arg]);
        } else if (strcmp(argv[arg], "-zoom") == 0) {
            arg++;
            g_zoom = (IceTFloat)atof(argv[arg]);
        } else if (strcmp(argv[arg], "-transparent") == 0) {
            g_transparent = ICET_TRUE;
        } else if (strcmp(argv[arg], "-colored-background") == 0) {
            g_colored_background = ICET_TRUE;
        } else if (strcmp(argv[arg], "-no-interlace") == 0) {
            g_no_interlace = ICET_TRUE;
        } else if (strcmp(argv[arg], "-no-collect") == 0) {
            g_no_collect = ICET_TRUE;
        } else if (strcmp(argv[arg], "-use-callback") == 0) {
            g_use_callback = ICET_TRUE;
        } else if (strcmp(argv[arg], "-dense-images") == 0) {
            g_dense_images = ICET_TRUE;
            /* Turn of interlacing. It won't help here. */
            g_no_interlace = ICET_TRUE;
        } else if (strcmp(argv[arg], "-sync-render") == 0) {
            g_sync_render = ICET_TRUE;
        } else if (strcmp(argv[arg], "-write-image") == 0) {
            g_write_image = ICET_TRUE;
        } else if (strcmp(argv[arg], "-reduce") == 0) {
            g_strategy = ICET_STRATEGY_REDUCE;
        } else if (strcmp(argv[arg], "-vtree") == 0) {
            g_strategy = ICET_STRATEGY_VTREE;
        } else if (strcmp(argv[arg], "-sequential") == 0) {
            g_strategy = ICET_STRATEGY_SEQUENTIAL;
        } else if (strcmp(argv[arg], "-bswap") == 0) {
            g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_BSWAP;
        } else if (strcmp(argv[arg], "-bswapfold") == 0) {
            g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_BSWAP_FOLDING;
        } else if (strcmp(argv[arg], "-radixk") == 0) {
            g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_RADIXK;
        } else if (strcmp(argv[arg], "-radixkr") == 0) {
            g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_RADIXKR;
        } else if (strcmp(argv[arg], "-tree") == 0) {
            g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_TREE;
        } else if (strcmp(argv[arg], "-magic-k-study") == 0) {
            g_do_magic_k_study = ICET_TRUE;
            g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_RADIXKR;
            arg++;
            g_max_magic_k = atoi(argv[arg]);
        } else if (strcmp(argv[arg], "-max-image-split-study") == 0) {
            g_do_image_split_study = ICET_TRUE;
            g_single_image_strategy = ICET_SINGLE_IMAGE_STRATEGY_RADIXKR;
            arg++;
            g_min_image_split = atoi(argv[arg]);
        } else if (strcmp(argv[arg], "-scaling-study-factor-2") == 0) {
            g_do_scaling_study_factor_2 = ICET_TRUE;
        } else if (strcmp(argv[arg], "-scaling-study-factor-2-3") == 0) {
            g_do_scaling_study_factor_2_3 = ICET_TRUE;
        } else if (strcmp(argv[arg], "-scaling-study-random") == 0) {
            arg++;
            g_num_scaling_study_random = atoi(argv[arg]);
        } else if (   (strcmp(argv[arg], "-h") == 0)
                   || (strcmp(argv[arg], "-help")) ) {
            usage(argv);
            exit(0);
        } else {
            printstat("Unknown option `%s'.\n", argv[arg]);
            usage(argv);
            exit(1);
        }
    }
}

#define NUM_HEX_PLANES 6
struct hexahedron {
    IceTDouble planes[NUM_HEX_PLANES][4];
};

static void intersect_ray_plane(const IceTDouble *ray_origin,
                                const IceTDouble *ray_direction,
                                const IceTDouble *plane,
                                IceTDouble *distance,
                                IceTBoolean *front_facing,
                                IceTBoolean *parallel)
{
    IceTDouble distance_numerator = icetDot3(plane, ray_origin) + plane[3];
    IceTDouble distance_denominator = icetDot3(plane, ray_direction);

    if (distance_denominator == 0.0) {
        *parallel = ICET_TRUE;
        *front_facing = (distance_numerator > 0);
    } else {
        *parallel = ICET_FALSE;
        *distance = -distance_numerator/distance_denominator;
        *front_facing = (distance_denominator < 0);
    }
}

/* This algorithm (and associated intersect_ray_plane) come from Graphics Gems
 * II, Fast Ray-Convex Polyhedron Intersection by Eric Haines. */
static void intersect_ray_hexahedron(const IceTDouble *ray_origin,
                                     const IceTDouble *ray_direction,
                                     const struct hexahedron hexahedron,
                                     IceTDouble *near_distance,
                                     IceTDouble *far_distance,
                                     IceTInt *near_plane_index,
                                     IceTBoolean *intersection_happened)
{
    int planeIdx;

    *near_distance = 0.0;
    *far_distance = 2.0;
    *near_plane_index = -1;

    for (planeIdx = 0; planeIdx < NUM_HEX_PLANES; planeIdx++) {
        IceTDouble distance;
        IceTBoolean front_facing;
        IceTBoolean parallel;

        intersect_ray_plane(ray_origin,
                            ray_direction,
                            hexahedron.planes[planeIdx],
                            &distance,
                            &front_facing,
                            &parallel);

        if (!parallel) {
            if (front_facing) {
                if (*near_distance < distance) {
                    *near_distance = distance;
                    *near_plane_index = planeIdx;
                }
            } else {
                if (distance < *far_distance) {
                    *far_distance = distance;
                }
            }
        } else { /*parallel*/
            if (front_facing) {
                /* Ray missed parallel plane.  No intersection. */
                *intersection_happened = ICET_FALSE;
                return;
            }
        }
    }

    *intersection_happened = (*near_distance < *far_distance);
}

/* Plane equations for unit box on origin. */
struct hexahedron unit_box = {
    {
        { -1.0, 0.0, 0.0, -0.5 },
        { 1.0, 0.0, 0.0, -0.5 },
        { 0.0, -1.0, 0.0, -0.5 },
        { 0.0, 1.0, 0.0, -0.5 },
        { 0.0, 0.0, -1.0, -0.5 },
        { 0.0, 0.0, 1.0, -0.5 }
    }
};

static void draw(const IceTDouble *projection_matrix,
                 const IceTDouble *modelview_matrix,
                 const IceTFloat *background_color,
                 const IceTInt *readback_viewport,
                 IceTImage result)
{
    IceTDouble transform[16];
    IceTDouble inverse_transpose_transform[16];
    IceTBoolean success;
    int planeIdx;
    struct hexahedron transformed_box;
    IceTInt width;
    IceTInt height;
    IceTFloat *colors_float = NULL;
    IceTUByte *colors_byte = NULL;
    IceTFloat *depths = NULL;
    IceTInt pixel_x;
    IceTInt pixel_y;
    IceTDouble ray_origin[3];
    IceTDouble ray_direction[3];
    IceTFloat background_depth;
    IceTFloat background_alpha;

    icetMatrixMultiply(transform, projection_matrix, modelview_matrix);

    success = icetMatrixInverseTranspose((const IceTDouble *)transform,
                                         inverse_transpose_transform);

    if (!success) {
        printrank("ERROR: Inverse failed.\n");
    }

    for (planeIdx = 0; planeIdx < NUM_HEX_PLANES; planeIdx++) {
        const IceTDouble *original_plane = unit_box.planes[planeIdx];
        IceTDouble *transformed_plane = transformed_box.planes[planeIdx];

        icetMatrixVectorMultiply(transformed_plane,
                                 inverse_transpose_transform,
                                 original_plane);
    }

    width = icetImageGetWidth(result);
    height = icetImageGetHeight(result);

    if (g_transparent) {
        colors_float = icetImageGetColorf(result);
    } else {
        colors_byte = icetImageGetColorub(result);
        depths = icetImageGetDepthf(result);
    }

    if (!g_dense_images) {
        background_depth = 1.0f;
        background_alpha = background_color[3];
    } else {
        IceTSizeType pixel_index;

        /* To fake dense images, use a depth and alpha for the background that
         * IceT will not recognize as background. */
        background_depth = 0.999f;
        background_alpha
                = (background_color[3] == 0) ? 0.001 : background_color[3];

        /* Clear out the the images to background so that pixels outside of
         * the contained viewport have valid values. */
        for (pixel_index = 0; pixel_index < width*height; pixel_index++) {
            if (g_transparent) {
                IceTFloat *color_dest = colors_float + 4*pixel_index;
                color_dest[0] = background_color[0];
                color_dest[1] = background_color[1];
                color_dest[2] = background_color[2];
                color_dest[3] = background_alpha;
            } else {
                IceTUByte *color_dest = colors_byte + 4*pixel_index;
                IceTFloat *depth_dest = depths + pixel_index;
                color_dest[0] = (IceTUByte)(background_color[0]*255);
                color_dest[1] = (IceTUByte)(background_color[1]*255);
                color_dest[2] = (IceTUByte)(background_color[2]*255);
                color_dest[3] = (IceTUByte)(background_alpha*255);
                depth_dest[0] = background_depth;
            }
        }
    }

    ray_direction[0] = ray_direction[1] = 0.0;
    ray_direction[2] = 1.0;
    ray_origin[2] = -1.0;
    for (pixel_y = readback_viewport[1];
         pixel_y < readback_viewport[1] + readback_viewport[3];
         pixel_y++) {
        ray_origin[1] = (2.0*pixel_y)/height - 1.0;
        for (pixel_x = readback_viewport[0];
             pixel_x < readback_viewport[0] + readback_viewport[2];
             pixel_x++) {
            IceTDouble near_distance;
            IceTDouble far_distance;
            IceTInt near_plane_index;
            IceTBoolean intersection_happened;
            IceTFloat color[4];
            IceTFloat depth;

            ray_origin[0] = (2.0*pixel_x)/width - 1.0;

            intersect_ray_hexahedron(ray_origin,
                                     ray_direction,
                                     transformed_box,
                                     &near_distance,
                                     &far_distance,
                                     &near_plane_index,
                                     &intersection_happened);

            if (intersection_happened) {
                const IceTDouble *near_plane;
                IceTDouble shading;

                near_plane = transformed_box.planes[near_plane_index];
                shading = -near_plane[2]/sqrt(icetDot3(near_plane, near_plane));

                color[0] = g_color[0] * (IceTFloat)shading;
                color[1] = g_color[1] * (IceTFloat)shading;
                color[2] = g_color[2] * (IceTFloat)shading;
                color[3] = g_color[3];
                depth = (IceTFloat)(0.5*near_distance);
                if (g_transparent) {
                    /* Modify color by an opacity determined by thickness. */
                    IceTDouble thickness = far_distance - near_distance;
                    IceTDouble opacity = QUICK_OPACITY(4.0*thickness);
                    if (opacity < 0.001) { opacity = 0.001; }
                    color[0] *= (IceTFloat)opacity;
                    color[1] *= (IceTFloat)opacity;
                    color[2] *= (IceTFloat)opacity;
                    color[3] *= (IceTFloat)opacity;
                }
            } else {
                color[0] = background_color[0];
                color[1] = background_color[1];
                color[2] = background_color[2];
                color[3] = background_alpha;
                depth = background_depth;
            }

            if (g_transparent) {
                IceTFloat *color_dest
                    = colors_float + 4*(pixel_y*width + pixel_x);
                color_dest[0] = color[0];
                color_dest[1] = color[1];
                color_dest[2] = color[2];
                color_dest[3] = color[3];
            } else {
                IceTUByte *color_dest
                    = colors_byte + 4*(pixel_y*width + pixel_x);
                IceTFloat *depth_dest
                    = depths + pixel_y*width + pixel_x;
                color_dest[0] = (IceTUByte)(color[0]*255);
                color_dest[1] = (IceTUByte)(color[1]*255);
                color_dest[2] = (IceTUByte)(color[2]*255);
                color_dest[3] = (IceTUByte)(color[3]*255);
                depth_dest[0] = depth;
            }
        }
    }

    if (g_first_render) {
        if (g_sync_render) {
            /* The rendering we are using here is pretty crummy.  It is not
               meant to be practical but to create reasonable images to
               composite.  One problem with it is that the render times are not
               well balanced even though everyone renders roughly the same sized
               object.  If you want to time the composite performance, this can
               interfere with the measurements.  To get around this problem, do
               a barrier that makes it look as if all rendering finishes at the
               same time.  Note that there is a remote possibility that not
               every process will render something, in which case this will
               deadlock.  Note that we make sure only to sync once to get around
               the less remote possibility that some, but not all, processes
               render more than once. */
            icetCommBarrier();
        }
        g_first_render = ICET_FALSE;
    }
}

/* Given the rank of this process in all of them, divides the unit box
 * centered on the origin evenly (w.r.t. volume) amongst all processes.  The
 * region for this process, characterized by the min and max corners, is
 * returned in the bounds_min and bounds_max parameters. */
static void find_region(int rank,
                        int num_proc,
                        float *bounds_min,
                        float *bounds_max,
                        region_divide *divisions)
{
    int axis = 0;
    int start_rank = 0;         /* The first rank. */
    int end_rank = num_proc;    /* One after the last rank. */
    region_divide current_division = NULL;

    bounds_min[0] = bounds_min[1] = bounds_min[2] = -0.5f;
    bounds_max[0] = bounds_max[1] = bounds_max[2] = 0.5f;

    *divisions = NULL;

    /* Recursively split each axis, dividing the number of processes in my group
       in half each time. */
    while (1 < (end_rank - start_rank)) {
        float length = bounds_max[axis] - bounds_min[axis];
        int middle_rank = (start_rank + end_rank)/2;
        float region_cut;
        region_divide new_divide = malloc(sizeof(struct region_divide_struct));

        /* Skew the place where we cut the region based on the relative size
         * of the group size on each side, which may be different if the
         * group cannot be divided evenly. */
        region_cut = (  bounds_min[axis]
                      + length*(middle_rank-start_rank)/(end_rank-start_rank) );

        new_divide->axis = axis;
        new_divide->cut = region_cut;
        new_divide->next = NULL;

        if (rank < middle_rank) {
            /* My rank is in the lower region. */
            new_divide->my_side = -1;
            new_divide->num_other_side = end_rank - middle_rank;
            bounds_max[axis] = region_cut;
            end_rank = middle_rank;
        } else {
            /* My rank is in the upper region. */
            new_divide->my_side = 1;
            new_divide->num_other_side = middle_rank - start_rank;
            bounds_min[axis] = region_cut;
            start_rank = middle_rank;
        }

        if (current_division != NULL) {
            current_division->next = new_divide;
        } else {
            *divisions = new_divide;
        }
        current_division = new_divide;

        axis = (axis + 1)%3;
    }
}

/* Free a region divide structure. */
static void free_region_divide(region_divide divisions)
{
    region_divide current_division = divisions;
    while (current_division != NULL) {
        region_divide next_division = current_division->next;
        free(current_division);
        current_division = next_division;
    }
}

/* Given the transformation matricies (representing camera position), determine
 * which side of each axis-aligned plane faces the camera.  The results are
 * stored in plane_orientations, which is expected to be an array of size 3.
 * Entry 0 in plane_orientations will be positive if the vector (1, 0, 0) points
 * towards the camera, negative otherwise.  Entries 1 and 2 are likewise for the
 * y and z vectors. */
static void get_axis_plane_orientations(const IceTDouble *projection,
                                        const IceTDouble *modelview,
                                        int *plane_orientations)
{
    IceTDouble full_transform[16];
    IceTDouble inverse_transpose_transform[16];
    IceTBoolean success;
    int planeIdx;

    icetMatrixMultiply(full_transform, projection, modelview);
    success = icetMatrixInverseTranspose((const IceTDouble *)full_transform,
                                         inverse_transpose_transform);

    for (planeIdx = 0; planeIdx < 3; planeIdx++) {
        IceTDouble plane_equation[4];
        IceTDouble transformed_plane[4];

        plane_equation[0] = plane_equation[1]
            = plane_equation[2] = plane_equation[3] = 0.0;
        plane_equation[planeIdx] = 1.0;

        /* To transform a plane, multiply the vector representing the plane
         * equation (ax + by + cz + d = 0) by the inverse transpose of the
         * transform. */
        icetMatrixVectorMultiply(transformed_plane,
                                 (const IceTDouble*)inverse_transpose_transform,
                                 (const IceTDouble*)plane_equation);

        /* If the normal of the plane is facing in the -z direction, then the
         * front of the plane is facing the camera. */
        if (transformed_plane[3] < 0) {
            plane_orientations[planeIdx] = 1;
        } else {
            plane_orientations[planeIdx] = -1;
        }
    }
}

/* Use the current OpenGL transformation matricies (representing camera
 * position) and the given region divisions to determine the composite
 * ordering. */
static void find_composite_order(const IceTDouble *projection,
                                 const IceTDouble *modelview,
                                 region_divide region_divisions)
{
    int num_proc = icetCommSize();
    IceTInt *process_ranks;
    IceTInt my_position;
    int plane_orientations[3];
    region_divide current_divide;

    get_axis_plane_orientations(projection, modelview, plane_orientations);

    my_position = 0;
    for (current_divide = region_divisions;
         current_divide != NULL;
         current_divide = current_divide->next) {
        int axis = current_divide->axis;
        int my_side = current_divide->my_side;
        int plane_side = plane_orientations[axis];
        /* If my_side is the side of the plane away from the camera, add
           everything on the other side as before me. */
        if (   ((my_side < 0) && (plane_side < 0))
            || ((0 < my_side) && (0 < plane_side)) ) {
            my_position += current_divide->num_other_side;
        }
    }

    process_ranks = malloc(num_proc * sizeof(IceTInt));
    icetCommAllgather(&my_position, 1, ICET_INT, process_ranks);

    icetEnable(ICET_ORDERED_COMPOSITE);
    icetCompositeOrder(process_ranks);

    free(process_ranks);
}

/* Finds the viewport of the bounds of the locally rendered geometry. */
/* This code is stolen from drawFindContainedViewport in draw.c. */
static void find_contained_viewport(IceTInt contained_viewport[4],
                                    const IceTDouble projection_matrix[16],
                                    const IceTDouble modelview_matrix[16])
{
    IceTDouble total_transform[16];
    IceTDouble left, right, bottom, top;
    IceTDouble *transformed_verts;
    IceTInt global_viewport[4];
    IceTInt num_bounding_verts;
    int i;

    icetGetIntegerv(ICET_GLOBAL_VIEWPORT, global_viewport);

    {
        IceTDouble viewport_matrix[16];
        IceTDouble tmp_matrix[16];

        /* Strange projection matrix that transforms the x and y of normalized
           screen coordinates into viewport coordinates that may be cast to
           integers. */
        viewport_matrix[ 0] = global_viewport[2];
        viewport_matrix[ 1] = 0.0;
        viewport_matrix[ 2] = 0.0;
        viewport_matrix[ 3] = 0.0;

        viewport_matrix[ 4] = 0.0;
        viewport_matrix[ 5] = global_viewport[3];
        viewport_matrix[ 6] = 0.0;
        viewport_matrix[ 7] = 0.0;

        viewport_matrix[ 8] = 0.0;
        viewport_matrix[ 9] = 0.0;
        viewport_matrix[10] = 2.0;
        viewport_matrix[11] = 0.0;

        viewport_matrix[12] = global_viewport[2] + global_viewport[0]*2.0;
        viewport_matrix[13] = global_viewport[3] + global_viewport[1]*2.0;
        viewport_matrix[14] = 0.0;
        viewport_matrix[15] = 2.0;

        icetMatrixMultiply(tmp_matrix,
                           (const IceTDouble *)projection_matrix,
                           (const IceTDouble *)modelview_matrix);
        icetMatrixMultiply(total_transform,
                           (const IceTDouble *)viewport_matrix,
                           (const IceTDouble *)tmp_matrix);
    }

    icetGetIntegerv(ICET_NUM_BOUNDING_VERTS, &num_bounding_verts);
    transformed_verts = icetGetStateBuffer(
                                       ICET_TRANSFORMED_BOUNDS,
                                       sizeof(IceTDouble)*num_bounding_verts*4);

    /* Transform each vertex to find where it lies in the global viewport and
       normalized z.  Leave the results in homogeneous coordinates for now. */
    {
        const IceTDouble *bound_vert
            = icetUnsafeStateGetDouble(ICET_GEOMETRY_BOUNDS);
        for (i = 0; i < num_bounding_verts; i++) {
            IceTDouble bound_vert_4vec[4];
            bound_vert_4vec[0] = bound_vert[3*i+0];
            bound_vert_4vec[1] = bound_vert[3*i+1];
            bound_vert_4vec[2] = bound_vert[3*i+2];
            bound_vert_4vec[3] = 1.0;
            icetMatrixVectorMultiply(transformed_verts + 4*i,
                                     (const IceTDouble *)total_transform,
                                     (const IceTDouble *)bound_vert_4vec);
        }
    }

    /* Set absolute mins and maxes. */
    left   = global_viewport[0] + global_viewport[2];
    right  = global_viewport[0];
    bottom = global_viewport[1] + global_viewport[3];
    top    = global_viewport[1];

    /* Now iterate over all the transformed verts and adjust the absolute mins
       and maxs to include them all. */
    for (i = 0; i < num_bounding_verts; i++)
    {
        IceTDouble *vert = transformed_verts + 4*i;

        /* Check to see if the vertex is in front of the near cut plane.  This
           is true when z/w >= -1 or z + w >= 0.  The second form is better just
           in case w is 0. */
        if (vert[2] + vert[3] >= 0.0) {
          /* Normalize homogeneous coordinates. */
            IceTDouble invw = 1.0/vert[3];
            IceTDouble x = vert[0]*invw;
            IceTDouble y = vert[1]*invw;

          /* Update contained region. */
            if (left   > x) left   = x;
            if (right  < x) right  = x;
            if (bottom > y) bottom = y;
            if (top    < y) top    = y;
        } else {
          /* The vertex is being clipped by the near plane.  In perspective
             mode, vertices behind the near clipping plane can sometimes give
             misleading projections.  Instead, find all the other vertices on
             the other side of the near plane, compute the intersection of the
             segment between the two points and the near plane (in homogeneous
             coordinates) and use that as the projection. */
            int j;
            for (j = 0; j < num_bounding_verts; j++) {
                IceTDouble *vert2 = transformed_verts + 4*j;
                double t;
                IceTDouble x, y, invw;
                if (vert2[2] + vert2[3] < 0.0) {
                  /* Ignore other points behind near plane. */
                    continue;
                }
              /* Let the two points in question be v_i and v_j.  Define the
                 segment between them with the parametric equation
                 p(t) = (vert - vert2)t + vert2.  First, find t where the z and
                 w coordinates of p(t) sum to zero. */
                t = (vert2[2]+vert2[3])/(vert2[2]-vert[2] + vert2[3]-vert[3]);
              /* Use t to find the intersection point.  While we are at it,
                 normalize the resulting coordinates.  We don't need z because
                 we know it is going to be -1. */
                invw = 1.0/((vert[3] - vert2[3])*t + vert2[3] );
                x = ((vert[0] - vert2[0])*t + vert2[0] ) * invw;
                y = ((vert[1] - vert2[1])*t + vert2[1] ) * invw;

              /* Update contained region. */
                if (left   > x) left   = x;
                if (right  < x) right  = x;
                if (bottom > y) bottom = y;
                if (top    < y) top    = y;
            }
        }
    }

    left = floor(left);
    right = ceil(right);
    bottom = floor(bottom);
    top = ceil(top);

  /* Clip bounds to global viewport. */
    if (left   < global_viewport[0]) left = global_viewport[0];
    if (right  > global_viewport[0] + global_viewport[2])
        right  = global_viewport[0] + global_viewport[2];
    if (bottom < global_viewport[1]) bottom = global_viewport[1];
    if (top    > global_viewport[1] + global_viewport[3])
        top    = global_viewport[1] + global_viewport[3];

  /* Use this information to build a containing viewport. */
    contained_viewport[0] = (IceTInt)left;
    contained_viewport[1] = (IceTInt)bottom;
    contained_viewport[2] = (IceTInt)(right - left);
    contained_viewport[3] = (IceTInt)(top - bottom);
}

static void SimpleTimingCollectAndPrintLog()
{
    IceTInt rank;
    IceTInt num_proc;
    IceTInt *log_sizes;

    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    /* Collect the number of log entries each process has. */
    log_sizes = malloc(num_proc*sizeof(IceTInt));
    icetCommGather(&g_timing_log_size, 1, ICET_SIZE_TYPE, log_sizes, 0);

    if (rank == 0) {
        timings_type *all_logs;
        IceTSizeType *data_sizes;
        IceTSizeType *offsets;
        IceTInt total_logs;
        IceTInt proc_index;
        IceTInt log_index;

        data_sizes = malloc(num_proc*sizeof(IceTSizeType));
        offsets = malloc(num_proc*sizeof(IceTSizeType));

        total_logs = 0;
        for (proc_index = 0; proc_index < num_proc; proc_index++) {
            data_sizes[proc_index] = log_sizes[proc_index]*sizeof(timings_type);
            offsets[proc_index] = total_logs*sizeof(timings_type);
            total_logs += log_sizes[proc_index];
        }

        all_logs = malloc(total_logs*sizeof(timings_type));
        icetCommGatherv(g_timing_log,
                        g_timing_log_size*sizeof(timings_type),
                        ICET_BYTE,
                        all_logs,
                        data_sizes,
                        offsets,
                        0);

        for (log_index = 0; log_index < total_logs; log_index++) {
            timings_type *timing = all_logs + log_index;
            printf("LOG,%d,%s,%s,%d,%d,%d,%d,%0.1f,%s,%s,%s,%s,%d,%d,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%lg,%ld,%lg\n",
                   timing->num_proc,
                   timing->strategy_name,
                   timing->si_strategy_name,
                   timing->num_tiles_x,
                   timing->num_tiles_y,
                   timing->screen_width,
                   timing->screen_height,
                   timing->zoom,
                   timing->transparent ? "yes" : "no",
                   timing->no_interlace ? "no" : "yes",
                   timing->no_collect ? "no" : "yes",
                   timing->dense_images ? "yes" : "no",
                   timing->max_image_split,
                   timing->frame_number,
                   timing->render_time,
                   timing->buffer_read_time,
                   timing->buffer_write_time,
                   timing->compress_time,
                   timing->interlace_time,
                   timing->blend_time,
                   timing->draw_time,
                   timing->composite_time,
                   timing->collect_time,
                   (long int)timing->bytes_sent,
                   timing->frame_time);
        }

        free(data_sizes);
        free(offsets);
        free(all_logs);
    } else /* rank != 0 */ {
        icetCommGatherv(g_timing_log,
                        g_timing_log_size*sizeof(timings_type),
                        ICET_BYTE,
                        NULL,
                        NULL,
                        NULL,
                        0);
    }

    free(log_sizes);

    if (g_timing_log_size > 0) {
        free(g_timing_log);
        g_timing_log = NULL;
        g_timing_log_size = 0;
    }

    /* This is to prevent a non-root from printing while the root is writing
       the log. */
    icetCommBarrier();
}

static int SimpleTimingDoRender()
{
    IceTInt rank;
    IceTInt num_proc;
    const char *strategy_name;
    const char *si_strategy_name;
    IceTInt max_image_split;

    float aspect = (  (float)(g_num_tiles_x*SCREEN_WIDTH)
                    / (float)(g_num_tiles_y*SCREEN_HEIGHT) );
    int frame;
    float bounds_min[3];
    float bounds_max[3];
    region_divide region_divisions;

    IceTDouble projection_matrix[16];
    IceTFloat background_color[4];

    IceTImage pre_rendered_image = icetImageNull();
    void *pre_rendered_image_buffer = NULL;

    timings_type *timing_array;

    /* Normally, the first thing that you do is set up your communication and
     * then create at least one IceT context.  This has already been done in the
     * calling function (i.e. icetTests_mpi.c).  See the init_mpi in
     * test_mpi.h for an example.
     */

    init_opacity_lookup();

    /* If we had set up the communication layer ourselves, we could have gotten
     * these parameters directly from it.  Since we did not, this provides an
     * alternate way. */
    icetGetIntegerv(ICET_RANK, &rank);
    icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);

    if (g_colored_background) {
        background_color[0] = 0.2f;
        background_color[1] = 0.5f;
        background_color[2] = 0.7f;
        background_color[3] = 1.0f;
    } else {
        background_color[0] = 0.0f;
        background_color[1] = 0.0f;
        background_color[2] = 0.0f;
        background_color[3] = 0.0f;
    }

    /* Give IceT a function that will issue the drawing commands. */
    icetDrawCallback(draw);

    /* Other IceT state. */
    if (g_transparent) {
        icetCompositeMode(ICET_COMPOSITE_MODE_BLEND);
        icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_FLOAT);
        icetSetDepthFormat(ICET_IMAGE_DEPTH_NONE);
        icetEnable(ICET_CORRECT_COLORED_BACKGROUND);
    } else {
        icetCompositeMode(ICET_COMPOSITE_MODE_Z_BUFFER);
        icetSetColorFormat(ICET_IMAGE_COLOR_RGBA_UBYTE);
        icetSetDepthFormat(ICET_IMAGE_DEPTH_FLOAT);
    }

    if (g_no_interlace) {
        icetDisable(ICET_INTERLACE_IMAGES);
    } else {
        icetEnable(ICET_INTERLACE_IMAGES);
    }

    if (g_no_collect) {
        icetDisable(ICET_COLLECT_IMAGES);
    } else {
        icetEnable(ICET_COLLECT_IMAGES);
    }

    /* Give IceT the bounds of the polygons that will be drawn.  Note that
         * IceT will take care of any transformation that gets passed to
         * icetDrawFrame. */
    icetBoundingBoxd(-0.5f, 0.5f, -0.5, 0.5, -0.5, 0.5);

    /* Determine the region we want the local geometry to be in.  This will be
     * used for the modelview transformation later. */
    find_region(rank, num_proc, bounds_min, bounds_max, &region_divisions);

    /* Set up the tiled display.  The asignment of displays to processes is
     * arbitrary because, as this is a timing test, I am not too concerned
     * about who shows what. */
    if (g_num_tiles_x*g_num_tiles_y <= num_proc) {
        int x, y, display_rank;
        icetResetTiles();
        display_rank = 0;
        for (y = 0; y < g_num_tiles_y; y++) {
            for (x = 0; x < g_num_tiles_x; x++) {
                icetAddTile(x*(IceTInt)SCREEN_WIDTH,
                            y*(IceTInt)SCREEN_HEIGHT,
                            SCREEN_WIDTH,
                            SCREEN_HEIGHT,
                            display_rank);
                display_rank++;
            }
        }
    } else {
        printstat("Not enough processes to %dx%d tiles.\n",
               g_num_tiles_x, g_num_tiles_y);
        return TEST_FAILED;
    }

    if (!g_use_callback) {
        IceTInt global_viewport[4];
        IceTInt width, height;
        IceTInt buffer_size;

        icetGetIntegerv(ICET_GLOBAL_VIEWPORT, global_viewport);
        width = global_viewport[2]; height = global_viewport[3];

        buffer_size = icetImageBufferSize(width, height);
        pre_rendered_image_buffer = malloc(buffer_size);
        pre_rendered_image =
                icetImageAssignBuffer(pre_rendered_image_buffer, width, height);
    }

    icetStrategy(g_strategy);
    icetSingleImageStrategy(g_single_image_strategy);

    /* Set up the projection matrix. */
    icetMatrixFrustum(-0.65*aspect/g_zoom, 0.65*aspect/g_zoom,
                      -0.65/g_zoom, 0.65/g_zoom,
                      3.0, 5.0,
                      projection_matrix);

    if (rank%10 < 7) {
        IceTInt color_bits = rank%10 + 1;
        g_color[0] = (float)(color_bits%2);
        g_color[1] = (float)((color_bits/2)%2);
        g_color[2] = (float)((color_bits/4)%2);
        g_color[3] = 1.0f;
    } else {
        g_color[0] = g_color[1] = g_color[2] = 0.5f;
        g_color[rank%10 - 7] = 0.0f;
        g_color[3] = 1.0f;
    }

    /* Initialize randomness. */
    if (rank == 0) {
        int i;
        printstat("Seed = %d\n", g_seed);
        for (i = 1; i < num_proc; i++) {
            icetCommSend(&g_seed, 1, ICET_INT, i, 33);
        }
    } else {
        icetCommRecv(&g_seed, 1, ICET_INT, 0, 33);
    }

    srand(g_seed);

    timing_array = malloc(g_num_frames * sizeof(timings_type));

    strategy_name = icetGetStrategyName();
    if (g_single_image_strategy == ICET_SINGLE_IMAGE_STRATEGY_RADIXK) {
        static char name_buffer[256];
        IceTInt magic_k;

        icetGetIntegerv(ICET_MAGIC_K, &magic_k);
        sprintf(name_buffer, "radix-k %d", (int)magic_k);
        si_strategy_name = name_buffer;
    } else if (g_single_image_strategy == ICET_SINGLE_IMAGE_STRATEGY_RADIXKR) {
            static char name_buffer[256];
            IceTInt magic_k;

            icetGetIntegerv(ICET_MAGIC_K, &magic_k);
            sprintf(name_buffer, "radix-kr %d", (int)magic_k);
            si_strategy_name = name_buffer;
    } else {
        si_strategy_name = icetGetSingleImageStrategyName();
    }

    icetGetIntegerv(ICET_MAX_IMAGE_SPLIT, &max_image_split);

    for (frame = 0; frame < g_num_frames; frame++) {
        IceTDouble elapsed_time;
        IceTDouble modelview_matrix[16];
        IceTImage image;

        /* We can set up a modelview matrix here and IceT will factor this in
         * determining the screen projection of the geometry. */
        icetMatrixIdentity(modelview_matrix);

        /* Move geometry back so that it can be seen by the camera. */
        icetMatrixMultiplyTranslate(modelview_matrix, 0.0, 0.0, -4.0);

        /* Rotate to some random view. */
        icetMatrixMultiplyRotate(modelview_matrix,
                                 (360.0*rand())/RAND_MAX, 1.0, 0.0, 0.0);
        icetMatrixMultiplyRotate(modelview_matrix,
                                 (360.0*rand())/RAND_MAX, 0.0, 1.0, 0.0);
        icetMatrixMultiplyRotate(modelview_matrix,
                                 (360.0*rand())/RAND_MAX, 0.0, 0.0, 1.0);

        /* Determine view ordering of geometry based on camera position
           (represented by the current projection and modelview matrices). */
        if (g_transparent) {
            find_composite_order(projection_matrix,
                                 modelview_matrix,
                                 region_divisions);
        }

        /* Translate the unit box centered on the origin to the region specified
         * by bounds_min and bounds_max. */
        icetMatrixMultiplyTranslate(modelview_matrix,
                                    bounds_min[0],
                                    bounds_min[1],
                                    bounds_min[2]);
        icetMatrixMultiplyScale(modelview_matrix,
                                bounds_max[0] - bounds_min[0],
                                bounds_max[1] - bounds_min[1],
                                bounds_max[2] - bounds_min[2]);
        icetMatrixMultiplyTranslate(modelview_matrix, 0.5, 0.5, 0.5);

        if (!g_use_callback) {
            /* Draw the image for the frame. */
            IceTInt contained_viewport[4];
            find_contained_viewport(contained_viewport,
                                    projection_matrix,
                                    modelview_matrix);
            if (g_transparent) {
                IceTFloat black[4] = { 0.0, 0.0, 0.0, 0.0 };
                draw(projection_matrix,
                     modelview_matrix,
                     black,
                     contained_viewport,
                     pre_rendered_image);
            } else {
                draw(projection_matrix,
                     modelview_matrix,
                     background_color,
                     contained_viewport,
                     pre_rendered_image);
            }
        }

        if (g_dense_images) {
            /* With dense images, we want IceT to load in all pixels, so clear
             * out the bounding box/vertices. */
            icetBoundingVertices(0, ICET_VOID, 0, 0, NULL);
        }

        /* Get everyone to start at the same time. */
        icetCommBarrier();

        elapsed_time = icetWallTime();

        if (g_use_callback) {
            /* Instead of calling draw() directly, call it indirectly through
             * icetDrawFrame().  IceT will automatically handle image
             * compositing. */
            g_first_render = ICET_TRUE;
            image = icetDrawFrame(projection_matrix,
                                  modelview_matrix,
                                  background_color);
        } else {
            image = icetCompositeImage(
                        icetImageGetColorConstVoid(pre_rendered_image,NULL),
                        g_transparent ? NULL : icetImageGetDepthConstVoid(pre_rendered_image,NULL),
                        NULL,
                        projection_matrix,
                        modelview_matrix,
                        background_color);
        }

        /* Let everyone catch up before finishing the frame. */
        icetCommBarrier();

        elapsed_time = icetWallTime() - elapsed_time;

        /* Record timings to logging. */
        timing_array[frame].num_proc = num_proc;
        strncpy(timing_array[frame].strategy_name, strategy_name, NAME_SIZE);
        timing_array[frame].strategy_name[NAME_SIZE-1] = '\0';
        strncpy(timing_array[frame].si_strategy_name, si_strategy_name, NAME_SIZE);
        timing_array[frame].si_strategy_name[NAME_SIZE-1] = '\0';
        timing_array[frame].num_tiles_x = g_num_tiles_x;
        timing_array[frame].num_tiles_y = g_num_tiles_y;
        timing_array[frame].screen_width = SCREEN_WIDTH;
        timing_array[frame].screen_height = SCREEN_HEIGHT;
        timing_array[frame].zoom = g_zoom;
        timing_array[frame].transparent = g_transparent;
        timing_array[frame].no_interlace = g_no_interlace;
        timing_array[frame].no_collect = g_no_collect;
        timing_array[frame].dense_images = g_dense_images;
        timing_array[frame].max_image_split = max_image_split;
        timing_array[frame].frame_number = frame;
        icetGetDoublev(ICET_RENDER_TIME,
                       &timing_array[frame].render_time);
        icetGetDoublev(ICET_BUFFER_READ_TIME,
                       &timing_array[frame].buffer_read_time);
        icetGetDoublev(ICET_BUFFER_WRITE_TIME,
                       &timing_array[frame].buffer_write_time);
        icetGetDoublev(ICET_COMPRESS_TIME,
                       &timing_array[frame].compress_time);
        icetGetDoublev(ICET_INTERLACE_TIME,
                       &timing_array[frame].interlace_time);
        icetGetDoublev(ICET_BLEND_TIME,
                       &timing_array[frame].blend_time);
        icetGetDoublev(ICET_TOTAL_DRAW_TIME,
                       &timing_array[frame].draw_time);
        icetGetDoublev(ICET_COMPOSITE_TIME,
                       &timing_array[frame].composite_time);
        icetGetDoublev(ICET_COLLECT_TIME,
                       &timing_array[frame].collect_time);
        timing_array[frame].bytes_sent
                = icetUnsafeStateGetInteger(ICET_BYTES_SENT)[0];
        timing_array[frame].frame_time = elapsed_time;

        /* Write out image to verify rendering occurred correctly. */
        if (   g_write_image
            && (rank < (g_num_tiles_x*g_num_tiles_y))
            && (frame == 0)
               ) {
            IceTUByte *buffer = malloc(SCREEN_WIDTH*SCREEN_HEIGHT*4);
            char filename[256];
            icetImageCopyColorub(image, buffer, ICET_IMAGE_COLOR_RGBA_UBYTE);
            sprintf(filename, "SimpleTiming%02d.ppm", rank);
            write_ppm(filename, buffer, (int)SCREEN_WIDTH, (int)SCREEN_HEIGHT);
            free(buffer);
        }
    }

    /* Collect max times and log. */
    {
        timings_type *timing_collection = malloc(num_proc*sizeof(timings_type));

        if (rank == 0) {
            if (g_timing_log_size == 0) {
                g_timing_log = malloc(g_num_frames*sizeof(timings_type));
            } else {
                g_timing_log = realloc(g_timing_log,
                                       (g_timing_log_size+g_num_frames)
                                        *sizeof(timings_type));
            }
        }

        for (frame = 0; frame < g_num_frames; frame++) {
            timings_type *timing = &timing_array[frame];

            icetCommGather(timing,
                           sizeof(timings_type),
                           ICET_BYTE,
                           timing_collection,
                           0);

            if (rank == 0) {
                int p;
                IceTInt64 total_bytes_sent = 0;

                for (p = 0; p < num_proc; p++) {
#define UPDATE_MAX(field) if (timing->field < timing_collection[p].field) timing->field = timing_collection[p].field;
                    UPDATE_MAX(render_time);
                    UPDATE_MAX(buffer_read_time);
                    UPDATE_MAX(buffer_write_time);
                    UPDATE_MAX(compress_time);
                    UPDATE_MAX(interlace_time);
                    UPDATE_MAX(blend_time);
                    UPDATE_MAX(draw_time);
                    UPDATE_MAX(composite_time);
                    UPDATE_MAX(collect_time);
                    UPDATE_MAX(frame_time);
                    total_bytes_sent += timing_collection[p].bytes_sent;
                }
                timing->bytes_sent = total_bytes_sent;

                g_timing_log[g_timing_log_size] = *timing;
                g_timing_log_size++;
            }
        }

        free(timing_collection);
    }

    free_region_divide(region_divisions);
    free(timing_array);

    pre_rendered_image = icetImageNull();
    if (pre_rendered_image_buffer != NULL) {
        free(pre_rendered_image_buffer);
    }

    return TEST_PASSED;
}

static int SimpleTimingDoParameterStudies()
{
    if (g_do_magic_k_study) {
        IceTContext original_context = icetGetContext();
        IceTInt magic_k;
        for (magic_k = 2; magic_k <= g_max_magic_k; magic_k *= 2) {
            char k_string[64];
            int retval;

#ifdef _WIN32
            sprintf(k_string, "ICET_MAGIC_K=%d", magic_k);
            putenv(k_string);
#else
            sprintf(k_string, "%d", magic_k);
            setenv("ICET_MAGIC_K", k_string, ICET_TRUE);
#endif

            /* This is a bit hackish.  The magic k value is set when the IceT
               context is initialized.  Thus, for the environment to take
               effect, we need to make a new context.  (Another benefit:
               resetting buffers.)  To make a new context, we need to get the
               communiator. */
            {
                IceTCommunicator comm = icetGetCommunicator();
                icetCreateContext(comm);
            }

            retval = SimpleTimingDoRender();

            /* We no longer need the context we just created. */
            icetDestroyContext(icetGetContext());
            icetSetContext(original_context);

            if (retval != TEST_PASSED) { return retval; }
        }
        return TEST_PASSED;
    } else if (g_do_image_split_study) {
        IceTContext original_context = icetGetContext();
        IceTInt num_proc;
        IceTInt magic_k;
        IceTInt image_split;

        icetGetIntegerv(ICET_NUM_PROCESSES, &num_proc);
        icetGetIntegerv(ICET_MAGIC_K, &magic_k);

        for (image_split = g_min_image_split;
             image_split <= num_proc;
             image_split *= 2) {
            char image_split_string[64];
            int retval;

#ifdef _WIN32
            sprintf(image_split_string, "ICET_MAX_IMAGE_SPLIT=%d", image_split);
            putenv(image_split_string);
#else
            sprintf(image_split_string, "%d", image_split);
            setenv("ICET_MAX_IMAGE_SPLIT", image_split_string, ICET_TRUE);
#endif

            /* This is a bit hackish.  The max image split value is set when the
               IceT context is initialized.  Thus, for the environment to take
               effect, we need to make a new context.  (Another benefit:
               resetting buffers.)  To make a new context, we need to get the
               communiator. */
            {
                IceTCommunicator comm = icetGetCommunicator();
                icetCreateContext(comm);
            }

            retval = SimpleTimingDoRender();

            /* We no longer need the context we just created. */
            icetDestroyContext(icetGetContext());
            icetSetContext(original_context);

            if (retval != TEST_PASSED) { return retval; }
        }
        return TEST_PASSED;
    } else {
        return SimpleTimingDoRender();
    }
}

static IceTCommunicator MakeCommSubset(IceTInt size, IceTInt offset)
{
    IceTInt32 *ranks;
    IceTInt rank_index;
    IceTCommunicator old_comm;
    IceTCommunicator new_comm;

    ranks = malloc(size*sizeof(IceTInt32));
    for (rank_index = 0; rank_index < size; rank_index++) {
        ranks[rank_index] = rank_index + offset;
    }

    old_comm = icetGetCommunicator();

    new_comm = old_comm->Subset(old_comm, size, ranks);

    free(ranks);

    return new_comm;
}

static int SimpleTimingDoScalingStudyFactor2()
{
    IceTInt size;
    IceTInt rank;
    IceTInt max_power_2;
    IceTInt min_size = g_num_tiles_x*g_num_tiles_y;
    IceTContext original_context = icetGetContext();
    int worst_result = TEST_PASSED;

    {
        int result = SimpleTimingDoParameterStudies();
        if (result != TEST_PASSED) { return result; }
    }
    SimpleTimingCollectAndPrintLog();

    icetGetIntegerv(ICET_NUM_PROCESSES, &size);
    icetGetIntegerv(ICET_RANK, &rank);
    max_power_2 = 1;
    while (max_power_2 <= size) { max_power_2 *= 2; }
    max_power_2 /= 2;

    if ((max_power_2 < size) && (max_power_2 >= min_size)) {
        IceTCommunicator new_communicator = MakeCommSubset(max_power_2, 0);
        if (rank < max_power_2) {
            IceTContext new_context = icetCreateContext(new_communicator);
            int result = SimpleTimingDoParameterStudies();
            if (result != TEST_PASSED) { worst_result = result; }
            icetSetContext(original_context);
            icetDestroyContext(new_context);
            new_communicator->Destroy(new_communicator);
        }
    }

    {
        IceTInt power_2 = max_power_2/2;
        IceTInt offset = 0;
        IceTBoolean has_group = ICET_FALSE;
        IceTCommunicator new_comm;
        while (power_2 >= min_size) {
            IceTCommunicator try_comm = MakeCommSubset(power_2, offset);
            if ((rank >= offset) && (rank < offset+power_2)) {
                has_group = ICET_TRUE;
                new_comm = try_comm;
            }
            offset += power_2;
            power_2 /= 2;
        }
        if (has_group) {
            IceTContext new_context;
            int result;
            new_context = icetCreateContext(new_comm);
            new_comm->Destroy(new_comm);

            result = SimpleTimingDoParameterStudies();
            if (result != TEST_PASSED) { worst_result = result; }

            icetSetContext(original_context);
            icetDestroyContext(new_context);
        }
    }

    return worst_result;
}

static int SimpleTimingDoScalingStudyFactor2_3()
{
    IceTInt size;
    IceTInt rank;
    IceTInt max_power_3;
    IceTInt min_size = g_num_tiles_x*g_num_tiles_y;
    IceTContext original_context = icetGetContext();
    int worst_result = TEST_PASSED;

    worst_result = SimpleTimingDoScalingStudyFactor2();
    SimpleTimingCollectAndPrintLog();

    icetGetIntegerv(ICET_NUM_PROCESSES, &size);
    icetGetIntegerv(ICET_RANK, &rank);
    max_power_3 = 1;
    while (max_power_3 <= size) { max_power_3 *= 3; }
    max_power_3 /= 3;

    if ((max_power_3*2 < size) && (max_power_3*2 >= min_size)) {
        IceTCommunicator new_communicator = MakeCommSubset(max_power_3*2, 0);
        if (rank < max_power_3*2) {
            IceTContext new_context = icetCreateContext(new_communicator);
            int result = SimpleTimingDoParameterStudies();
            if (result != TEST_PASSED) { worst_result = result; }
            icetSetContext(original_context);
            icetDestroyContext(new_context);
            new_communicator->Destroy(new_communicator);
        }
        SimpleTimingCollectAndPrintLog();
    }

    {
        // Start with a context with a power of three processes.
        IceTCommunicator new_comm = MakeCommSubset(max_power_3, 0);
        if (new_comm == ICET_COMM_NULL) {
            // This rank is not participating in the rest of the tests.
            return worst_result;
        }
        icetCreateContext(new_comm);
        new_comm->Destroy(new_comm);
    }

    if (max_power_3 < 3) {
        // Corner case where we are running with to few processes to make
        // any factors. The code below can break, so just bail.
        icetDestroyContext(icetGetContext());
        icetSetContext(original_context);
        return worst_result;
    }

    if ((max_power_3 < size) && (max_power_3 >= min_size)) {
        if (rank < max_power_3) {
            int result = SimpleTimingDoParameterStudies();
            if (result != TEST_PASSED) { worst_result = result; }
        }
        SimpleTimingCollectAndPrintLog();
    }

    // This loop will run in log_3 iterations selecting any factors of 2 and 3
    // it finds. Let us say max_power_3 = 3^N. The iterations of the loop
    // break up the processors is partitions of 1/3 and 2/3 as follows.
    // (Iteration 0 was just done above.)
    //
    // Iteration 0: |---------------------------3^N--------------------------|
    // Iteration 1: |------3^(N-1)-----|--------------2*3^(N-1)--------------|
    // Iteration 2: |3^(N-2)|2*3^(N-2)-|-(removed)-|--------4*3^(N-2)--------|
    // ...
    //
    // Note that some of the partitions are removed because they are repeates.
    // In the example above, the removed partition is of size 2*3^(N-2), which
    // is the same size of another partition in the tree. In general, when we
    // break up a partition that is not a power of 3, we only generate the
    // partition that is 2/3 because the 1/3 partition is created elsewhere.
    // A partition that is exactly a power of 3 (no factors of 2) has both
    // subpartitions created.
    //

    while (ICET_TRUE) {
        IceTInt last_size;
        IceTInt last_rank;
        IceTInt this_size;
        IceTCommunicator comm_third;
        IceTCommunicator comm_two_thirds;
        IceTContext old_context = icetGetContext();
        int result;

        icetGetIntegerv(ICET_NUM_PROCESSES, &last_size);
        icetGetIntegerv(ICET_RANK, &last_rank);
        this_size = last_size/3;

        if (this_size%3 != 0) {
            // Next smallest comms have no factors of 3. Must be only
            // factors of two, and we have done that.
            break;
        }

        // By simple factoring, we can split the last communicator into one
        // piece a third of its size and another peice 2/3 the size, and
        // those combined will use all the processes.
        comm_third = MakeCommSubset(this_size, 0);
        comm_two_thirds = MakeCommSubset(2*this_size, this_size);

        icetDestroyContext(old_context);
        if (last_rank < this_size) {
            icetCreateContext(comm_third);
            comm_third->Destroy(comm_third);
            if (this_size%2 == 0) {
                // If this size already has a factor of two, we can drop it.
                // See the description above.
                break;
            }
        } else {
            icetCreateContext(comm_two_thirds);
            comm_two_thirds->Destroy(comm_two_thirds);
            this_size *= 2;
        }

        if (this_size < min_size) {
            // Group has gotten too small to handle these tiles.
            break;
        }

        result = SimpleTimingDoParameterStudies();
        if (result != TEST_PASSED) { worst_result = result; }
    }

    icetDestroyContext(icetGetContext());
    icetSetContext(original_context);

    return worst_result;
}

static int SimpleTimingDoScalingStudyRandom()
{
    IceTInt size;
    IceTInt rank;
    IceTInt min_size = g_num_tiles_x*g_num_tiles_y;
    IceTContext original_context = icetGetContext();
    int worst_result = TEST_PASSED;
    IceTInt trial;
    IceTInt *pivots;

    icetGetIntegerv(ICET_NUM_PROCESSES, &size);
    icetGetIntegerv(ICET_RANK, &rank);

    /* Choose pivot points to bifurcate processes. Do them all at once here
     * so the psudorandom numbers do not interfear with those choosen during
     * the rendering. */
    pivots = malloc(sizeof(IceTInt)*g_num_scaling_study_random);
    srand(g_seed);
    for (trial = 0; trial < g_num_scaling_study_random; trial++) {
        pivots[trial] = rand()%size;
    }

    for (trial = 0; trial < g_num_scaling_study_random; trial++) {
        IceTCommunicator left_comm;
        IceTCommunicator right_comm;
        IceTInt left_size;
        IceTInt right_size;
        IceTInt local_size;
        int result;

        /* Print out results from last run. */
        SimpleTimingCollectAndPrintLog();

        left_size = pivots[trial];
        right_size = size-left_size;

        left_comm = MakeCommSubset(left_size, 0);
        right_comm = MakeCommSubset(right_size, left_size);

        if (rank < left_size) {
            icetCreateContext(left_comm);
            left_comm->Destroy(left_comm);
            local_size = left_size;
        } else {
            icetCreateContext(right_comm);
            right_comm->Destroy(right_comm);
            local_size = right_size;
        }

        if (local_size > min_size) {
            result = SimpleTimingDoParameterStudies();
            if (result != TEST_PASSED) { worst_result = result; }
        }

        icetDestroyContext(icetGetContext());
        icetSetContext(original_context);
    }

    free(pivots);

    return worst_result;
}

static int SimpleTimingDoScalingStudies()
{
    int result;
    if (g_do_scaling_study_factor_2_3) {
        result = SimpleTimingDoScalingStudyFactor2_3();
    } else if (g_do_scaling_study_factor_2) {
        result = SimpleTimingDoScalingStudyFactor2();
    } else {
        result = SimpleTimingDoParameterStudies();
    }

    if (g_num_scaling_study_random > 0) {
        int new_result = SimpleTimingDoScalingStudyRandom();
        if (new_result != TEST_PASSED) {
            result = new_result;
        }
    }

    SimpleTimingCollectAndPrintLog();
    return result;
}

int SimpleTimingRun()
{
    IceTInt rank;

    icetGetIntegerv(ICET_RANK, &rank);

    if (rank == 0) {
        printf("HEADER,"
               "num processes,"
               "multi-tile strategy,"
               "single-image strategy,"
               "tiles x,"
               "tiles y,"
               "width,"
               "height,"
               "zoom,"
               "transparent,"
               "interlacing,"
               "collection,"
               "dense images,"
               "max image split,"
               "frame,"
               "render time,"
               "buffer read time,"
               "buffer write time,"
               "compress time,"
               "interlace time,"
               "blend time,"
               "draw time,"
               "composite time,"
               "collect time,"
               "bytes sent,"
               "frame time\n");
    }

    g_timing_log = NULL;
    g_timing_log_size = 0;

    return SimpleTimingDoScalingStudies();
}

int SimpleTiming(int argc, char * argv[])
{
    parse_arguments(argc, argv);

    return run_test(SimpleTimingRun);
}
