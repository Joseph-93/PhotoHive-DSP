#include "filtering.h"
#include <stdlib.h>
#include <math.h>

#define THRESHOLD 0.2


/******************************************************************************
 * initialize_3x3_laplacian simply builds a laplacian filter of size 3x3.
******************************************************************************/
Filter* initialize_3x3_laplacian() {
    Filter* filt = (Filter*)malloc(sizeof(Filter));
    filt->height = 3;
    filt->width = 3;

    filt->coefs = (double*)malloc(9 * sizeof(double));
    filt->coefs[0] = -1.0; filt->coefs[1] = -1.0; filt->coefs[2] = -1.0;
    filt->coefs[3] = -1.0; filt->coefs[4] = 8; filt->coefs[5] = -1.0;
    filt->coefs[6] = -1.0; filt->coefs[7] = -1.0; filt->coefs[8] = -1.0;
    return filt;
}


/******************************************************************************
 * sharpness_avg calculates a heuristic for sharpness, by averaging the values
 *   of a laplacian-filtered image that are above a given THRESHOLD. Function
 *   attempts to replace the laplacian-variance sharpness measure.
******************************************************************************/
Pixel sharpness_avg(Pixel* input, int length) {
    Pixel p;
    Pixel p_total = 0.0;
    int num_p = 0;
    for (int i=0; i<length; i++) {
        p = input[i];
        if (p > THRESHOLD) {
            p_total += p;
            num_p++;
        }
    }
    Pixel avg = p_total/((Pixel)(num_p));
    return avg;
}


/******************************************************************************
 * filter_image runs a Pixel array through a filter and returns a pointer to
 *   its output array.
 *  -filt is the MxN filter object. Can be any size.
 *  -input is the input array.
 *  -height and width are the image's height and width.
******************************************************************************/
Pixel* filter_image(Filter* filt, Pixel* input, int height, int width) {
    Pixel* output = (Pixel*)malloc(height * width * sizeof(Pixel));
    int filt_h = filt->height;
    int filt_w = filt->width;
    int yoffs = filt_h / 2;
    int xoffs = filt_w / 2;

    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            double dotp = 0.0;
            for (int fy = 0; fy < filt_h; fy++) {
                for (int fx = 0; fx < filt_w; fx++) {
                    int iy = y + fy - yoffs;
                    int ix = x + fx - xoffs;

                    if (iy >= 0 && iy < height && ix >= 0 && ix < width) {
                        int img_idx = iy * width + ix;
                        int filt_idx = fy * filt_w + fx;
                        dotp += input[img_idx] * filt->coefs[filt_idx];
                    }
                }
            }
            output[y * width + x] = dotp;
        }
    }
    return output;
}


Image_RGB* create_filtered_RGB(Image_RGB* input, Filter* filt) {
    Image_RGB* output = create_rgb_image(input->width, input->height);
    output->r = filter_image(filt, input->r, output->height, output->width);
    output->g = filter_image(filt, input->g, output->height, output->width);
    output->b = filter_image(filt, input->b, output->height, output->width);
    return output;
}


/******************************************************************************
 * get_average gets the average value from an array of inputs and its length.
 *  -Function converts from Pixel to double to prevent round-to-nearest errors
 *   in accumulation between the massive accumulator and comparatively tiny
 *   input values.
******************************************************************************/
Pixel get_average(Pixel* input, int length) {
    double accumulator = 0;
    for (int i=0; i<length; i++) {
        accumulator += (double)input[i];
    }
    return (Pixel)(accumulator/(double)length);
}


/******************************************************************************
 * get_variance gets the variance of an array, given its length and average.
 *  -Function converts from Pixel to double to prevent round-to-nearest errors
 *   in accumulation between the massive accumulator and comparatively tiny
 *   input values.
******************************************************************************/
Pixel get_variance(Pixel* input, int length, Pixel average) {
    double accumulator = 0;
    for (int i=0; i<length; i++) {
        double diff = (double)(input[i]-average);
        accumulator += diff*diff;
    }
    accumulator = accumulator/(double)length;
    return accumulator;
}


Pixel get_variance_sharpness(Pixel* input, int height, int width) {
    // run laplacian kernel filter through image
    Filter* filt = initialize_3x3_laplacian();
    Pixel* filtered = filter_image(filt, input, height, width);

    // get variance
    int length = height*width;
    START_TIMING(laplacian_avg_time);
    Pixel avg = get_average(filtered, length);
    END_TIMING(laplacian_avg_time, "getting average of laplacian");

    START_TIMING(variance_time);
    Pixel variance = get_variance(filtered, length, avg);
    END_TIMING(variance_time, "getting the variance of laplacian");


    // Clean up memory
    free(filtered);
    free(filt->coefs);
    free(filt);
    return variance;
}


Pixel get_average_sharpness(Pixel* input, int height, int width) {
    // Run laplacian kernel filter through image
    Filter* filt = initialize_3x3_laplacian();
    Pixel* filtered = filter_image(filt, input, height, width);

    // Get sharpness average
    int length = height*width;
    Pixel sharpness_average = sharpness_avg(filtered, length);

    // Clean up memory
    free(filt->coefs);
    free(filt);
    free(filtered);
    return sharpness_average;
}
