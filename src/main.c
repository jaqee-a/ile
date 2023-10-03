#include "../include/cherry-graphics/lib/cherry.h"

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIDTH  850
#define HEIGHT 850

#define SAND 0xFFF2D398
#define DEEP_WATER 0xFF256EFF
#define WATER 0xFF0ACDFF
#define DIRT 0xFF685044

#define WHITE 0xFFFFFFFF
#define BLACK 0x00000000

static uint32_t pixels[WIDTH*HEIGHT];

uint32_t* grayscale(uint32_t* image, int32_t i_width, int32_t i_height)
{
    uint32_t* new_image = (uint32_t*)malloc(sizeof(uint32_t) * i_width * i_height);
    uint32_t t_r, t_g, t_b, avg;
    int32_t x, y;
    for(y=0; y<i_height; y+=1) {
        for(x=0; x<i_width; x+=1) {

            t_r= image[y*i_width+x]&0xFF;
            t_g=(image[y*i_width+x]>>8)&0xFF;
            t_b=(image[y*i_width+x]>>16)&0xFF;

            avg = (t_r + t_g + t_b) / 3;

            new_image[y*i_width+x] = avg+(avg<<8)+(avg<<16)+0xFF000000;

        }
    }
    return new_image;
}

void convolution(uint32_t* image, int32_t i_width, int32_t i_height, float* kernel, int32_t kernel_size)
{
    uint32_t* new_image = (uint32_t*)malloc(sizeof(uint32_t) * i_width * i_height);
    uint32_t sum, t_r, t_g, t_b;
    int32_t kernel_width = sqrt(kernel_size);
    int32_t mid_kernel = (kernel_width - 1) / 2;
    int32_t x, y, i, j;
    int32_t x_coord, y_coord;

    for(y=0; y<i_height; y+=1) {
        for(x=0; x<i_width; x+=1) {
            sum = 0;
                
            for(j=-mid_kernel; j<mid_kernel+1; ++j) {
                for(i=-mid_kernel; i<mid_kernel+1; ++i)
                {
                    x_coord = i+x;
                    y_coord = j+y;

                    if(x_coord < 0 || y_coord < 0 || x_coord >= i_width || y_coord >= i_height) continue;

                    t_r=(uint32_t)(( image[y_coord*i_width+x_coord]&0xFF) * kernel[j*kernel_width+i] / (float)kernel_size)%0xFF;
                    t_g=(uint32_t)(((image[y_coord*i_width+x_coord]>>8)&0xFF) * kernel[j*kernel_width+i] / (float)kernel_size)%0xFF;
                    t_b=(uint32_t)(((image[y_coord*i_width+x_coord]>>16)&0xFF) * kernel[j*kernel_width+i] / (float)kernel_size)%0xFF;

                    sum += t_r+(t_g<<8)+(t_b<<16)+0xFF000000;
                }
            }

            new_image[y*i_width+x] = sum;

        }
    }

    for(y=0; y<i_height; y+=1) {
        for(x=0; x<i_width; x+=1) {
            image[y*i_width+x] = new_image[y*i_width+x];
        }
    }

    free(new_image);
    // image = new_image;
}

typedef struct Point {
    int x, y;
} Point;

int32_t flood_fill(uint32_t* pixels, int8_t* visit_list, int32_t i_width, int32_t i_height, int32_t x, int32_t y) 
{
    if(x < 0 || y < 0 || x >= i_width || y >= i_height || visit_list[y*i_width+x] == 1 || pixels[y*i_width+x] != SAND) return 0;
    
    visit_list[y*i_width+x] = 1;

    return
        flood_fill(pixels, visit_list, i_width, i_height, x + 1, y) +
        flood_fill(pixels, visit_list, i_width, i_height, x - 1, y) +
        flood_fill(pixels, visit_list, i_width, i_height, x, y + 1) +
        flood_fill(pixels, visit_list, i_width, i_height, x, y - 1) + 1;
}

Point largest_island(uint32_t* pixels, int32_t i_width, int32_t i_height)
{
    int8_t* visit_list = (int8_t*)malloc(sizeof(uint32_t) * i_width * i_height);
    memset(visit_list, 0, sizeof(uint32_t) * i_width * i_height);
    int32_t x, y, max_island_size = 0, curr_island_size;
    Point pt = {0, 0};
    for(y=0; y<i_height; y+=1) {
        for(x=0; x<i_width; x+=1) {
            if(pixels[y*i_width+x] == SAND && visit_list[y*i_width+x] == 0) {
                curr_island_size = flood_fill(pixels, visit_list, i_width, i_height, x, y);
                if(curr_island_size > max_island_size) {
                    max_island_size = curr_island_size;
                    pt.x = x;
                    pt.y = y;
                }
            }
        }
    }
    free(visit_list);
    return pt;
}

void procedural_island()
{
    cherry_fill(pixels, WIDTH, HEIGHT, BLACK);

    float max_d=(WIDTH/2)*(WIDTH/2)+(HEIGHT/2)*(HEIGHT/2);
    float radius=0.25;

    int32_t x, y;
    uint32_t k, r, g, b;
    float dist_c;
    
    // Set sand / water
    for(y=0; y<HEIGHT; y+=1)
    for(x=0; x<WIDTH; x+=1)
    {
        float dx=(float)x-WIDTH/2;
        float dy=(float)y-HEIGHT/2;
        dist_c=((dx*dx)+(dy*dy))/max_d;
        float color=1-smoothstep(radius-0.3, radius+0.3, dist_c);
        float n = noise(x/100.0, y/100.0, 0)*color;
        int32_t v = n < 0.4 ? DEEP_WATER : SAND;
        pixels[y*WIDTH+x]=v;
    }

    // for(y=0; y<HEIGHT; y+=1)
    // for(x=0; x<WIDTH; x+=1)
    // {
    //     dist_c=(x-pt.x)*(x-pt.x)+(y-pt.y)*(y-pt.y);

    //     if(dist_c < 50) {
    //         pixels[y*WIDTH+x] = 0xFFFF0000;
    //     }
    // }

    // uint32_t* gc = grayscale(pixels, WIDTH, HEIGHT);

    // float kernel[9] = {0, 0.1, 0, 0, 0.1, 0, 0, 0.1, 0};
    // float kernel[9] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
    // float kernel[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
    // convolution(gc, WIDTH, HEIGHT, kernel, kernel_width);

    // for(int32_t j=0; j<HEIGHT; j+=1)
    // for(int32_t i=0; i<WIDTH; i+=1)
    // {
    //     k= gc[j*WIDTH+i]&0xFF;

    //     r=( pixels[j*WIDTH+i]&0xFF) * (float)k/255;
    //     g=((pixels[j*WIDTH+i]>>8)&0xFF) * (float)k/255;
    //     b=((pixels[j*WIDTH+i]>>16)&0xFF) * (float)k/255;

    //     pixels[j*WIDTH+i] = r+(g<<8)+(b<<16)+0xFF000000;
    // }
    // free(gc);
    int32_t kernel_width=9;

    if(!(kernel_width&1)) {
        fprintf(stderr, "Please select an odd number as a kernel width");
        exit(-1);
    }
    int32_t x_coord, y_coord;

    for(int32_t j=0; j<HEIGHT; j+=1)
    for(int32_t i=0; i<WIDTH; i+=1)
    {
        if(pixels[j*WIDTH+i]==SAND)

        // if((i-1 >= 0 && pixels[j*WIDTH+i-1] == SAND) ||
        //    (j-1 < HEIGHT && pixels[(j-1)*WIDTH+i] == SAND))

        for(y=-kernel_width; y<kernel_width; y+=1)
        for(x=-kernel_width; x<kernel_width; x+=1)
        {
            if(x == y) continue;
            
            x_coord = i+x;
            y_coord = j+y;

            if(x_coord < 0 || y_coord < 0 || x_coord >= WIDTH || y_coord >= HEIGHT) continue;

            dist_c=(x*x+y*y);
            if(dist_c < kernel_width*kernel_width && pixels[y_coord*WIDTH+x_coord] == DEEP_WATER) {
                pixels[y_coord*WIDTH+x_coord] = BLACK;
            }
            
        }
    }

    Point pt = largest_island(pixels, WIDTH, HEIGHT);

    cherry_fill_circle(pixels, WIDTH, HEIGHT, pt.x, pt.y, 20, 0xFFFF0000);

    // for(int32_t j=0; j<HEIGHT; j+=1)
    // for(int32_t i=0; i<WIDTH; i+=1)
    // {
    //     if(pixels[j*WIDTH+i]==0xFFF2D398)

    //     for(y=-kernel_width; y<kernel_width; y+=1)
    //     for(x=-kernel_width; x<kernel_width; x+=1)
    //     {
    //         if(x == y) continue;
    //         
    //         x_coord = i+x;
    //         y_coord = j+y;

    //         if(x_coord < 0 || y_coord < 0 || x_coord >= WIDTH || y_coord >= HEIGHT) continue;

    //         dist_c=(x*x+y*y);
    //         if(dist_c < kernel_width*kernel_width && pixels[y_coord*WIDTH+x_coord] == 0xFF256EFF) {
    //             pixels[y_coord*WIDTH+x_coord] = 0xFF0ACDFF;
    //         }
    //         
    //     }
    // }

    const char* file_path = "examples/island.bmp";
    Errno err = cherry_save_to_bmp(pixels, COLOR_MODE_BGR, WIDTH, HEIGHT, file_path);
    if(err) {
        fprintf(stderr, "Could write to file %s: %s", file_path, strerror(err));
    }

    printf("DONE");
}

int main(void)
{
    procedural_island();
    
    return 0;
}
