// cvdemo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "loadBMP.h"
#include "targetver.h"


void average_greyscale(IMAGE *img)
{
    printf("Converting to grayscale\n");
    for(uint32_t i = 0; i < (img->width*img->height); i++ ) 
	{
        uint8_t avg = (img->data[i].r + img->data[i].g + img->data[i].b)/3;
        img->data[i].r = avg;
        img->data[i].g = avg;
        img->data[i].b = avg;
    }
}

void  binary(IMAGE *img,int thr ) //固定阈值二值化
{
	printf("Converting to binary\n");
    for(uint32_t i = 0; i < (img->width*img->height); i++ ) 
	{
        uint8_t avg = (img->data[i].r + img->data[i].g + img->data[i].b)/3;
        if (avg < thr)
			img->data[i].b = img->data[i].g = img->data[i].r = 0;
		else
			img->data[i].b = img->data[i].g = img->data[i].r = 255;
    }
}

void  adaptivebinary(IMAGE *img )//自适应阈值二值化  Otsu
{
	int width = img->width;
	int height = img->height;
	int PixelCount[GrayScale] = { 0 };
	float PixelPro[GrayScale] = { 0 };
	int i, j, PixelSum = width*height;
	int threshold = 0;
	uint8_t* Data = (uint8_t*)img->data;

	for (i = 0; i < height; i++)		//求出每个灰度值对应的像素点数
	{
		for (j = 0; j < width; j++)
		{
			uint8_t data = (img->data[i*width + j].r + img->data[i*width + j].g + img->data[i*width + j].b) / 3;
			PixelCount[data]++;
		}
	}

	for (i = 0; i < GrayScale; i++)		//求出每个灰度值的像素点数的比例
	{
		PixelPro[i] = (float)PixelCount[i] / PixelSum;
	}

	float w0, w1, u0tmp, u1tmp, u0, u1, deltaTmp, deltaMax=0;
	for (i = 0; i < GrayScale; i++)		//遍历0到255寻求合适的阈值
	{	
		w0 = w1 = u0tmp = u1tmp = u0 = u1 = deltaTmp = 0;
		for (j = 0; j < GrayScale; j++)
		{
			if (j <= i)			//背景部分
			{
				w0 += PixelPro[j];
				u0tmp += j*PixelPro[j];
			}
			else               //前景部分    
			{
				w1 += PixelPro[j];
				u1tmp += j*PixelPro[j];
			}
		}
		u0 = u0tmp / w0;
		u1 = u1tmp / w1;
		deltaTmp = (float)(w0 *w1* pow((u0 - u1), 2));		//类间方差

		if (deltaTmp > deltaMax)
		{
			deltaMax = deltaTmp;
			threshold = i;
		}
	}
	

	for (i = 0; i < PixelSum; i++)
	{
		uint8_t avg = (img->data[i].b + img->data[i].g + img->data[i].r) / 3;
		if (avg>threshold)
		{
			img->data[i].b = img->data[i].g = img->data[i].r = 255;
		}
		else
		{
			img->data[i].b = img->data[i].g = img->data[i].r = 0;
		}
	}
}

void  gussianblur(IMAGE *img)//高斯模糊
{
	printf("Converting to gussianblur\n");
	uint32_t templates[9] = { 1, 2, 1,		//高斯核
						2, 4, 2,
						1, 2, 1};
	uint32_t num = img->height * img->width * sizeof(ColorRGB);
	ColorRGB *tmpdata = (ColorRGB*)malloc(num);		//拷贝图片信息
	memcpy(tmpdata, img->data, num);
	uint32_t index = 0, sum_r = 0, sum_g = 0, sum_b = 0, sum = 0, avg=0;
	for (uint32_t i = 1; i < img->height - 1; i++)
	{
		for (uint32_t j = 1; j < img->width - 1; j++)
		{
			index = sum_r = sum_g = sum_b = sum=0;
			for (uint32_t m = i - 1; m < i + 2; m++)
			{
				for (uint32_t n = j - 1; n < j + 2; n++)
				{
					avg = (tmpdata[m*img->width + n].r + tmpdata[m*img->width + n].g + tmpdata[m*img->width + n].b) / 3;
					sum += avg*templates[index];
					/*sum_r += tmpdata[m * img->width + n].b * templates[index];
					sum_g += tmpdata[m * img->width + n].g * templates[index];
					sum_b += tmpdata[m * img->width + n].r * templates[index];*/
					index++;
				}
			}
			/*img->data[i * img->width + j].r = sum_r / 16;
			img->data[i * img->width + j].g = sum_g / 16;
			img->data[i * img->width + j].b = sum_b / 16;*/
			img->data[i*img->width + j].r = img->data[i*img->width + j].g = img->data[i*img->width + j].b = sum / 16;
		}
	}

	free(tmpdata);
}

void  median(IMAGE *img,int xwin, int ywin  )//中值滤波 xwin ,ywin 分别是窗口的宽和高
{
	printf("Converting to median.\n");
	
	uint32_t sum = img->height * img->width * sizeof(ColorRGB);
	ColorRGB *tmpdata = (ColorRGB*)malloc(img->height * img->width * sizeof(ColorRGB));
	memcpy(tmpdata, img->data, sum);
	
	uint32_t size = xwin*ywin;
	/*int *matrix = (int*)malloc(size*sizeof(int));*/
	for (uint32_t i = 0; i < img->height - ywin; i++)     //(i,j)遍历所有中值点
	{
		for (uint32_t j = 0; j < img->width - xwin; j++)
		{
			uint32_t k = 0;
			uint32_t* matrix = (uint32_t*)malloc(size*sizeof(uint32_t));
			for (uint32_t m = i; m < i + ywin; m++)
			{
				for (uint32_t n = j; n < j + xwin; n++)	//(m,n)遍历窗口内的点
				{
					uint32_t p = m*img->width + n;
					matrix[k] = (tmpdata[p].r + tmpdata[p].g + tmpdata[p].b) / 3;
					k++;
				}
			}
			uint32_t temp, num;
			for (uint32_t m = 0; m < size - 1; m++)    //排序
			{
				for (uint32_t n = 0; n < size - m - 1; n++)
				{
					if (matrix[n]>matrix[n + 1])
					{
						temp = matrix[n];
						matrix[n] = matrix[n + 1];
						matrix[n + 1] = matrix[n];
					}
				}
			}

			if (0 == size / 2)			//取中值
				num = (matrix[size / 2] + matrix[size / 2 + 1]) / 2;
			else
				num = matrix[size / 2 + 1];

			img->data[i*img->width + j].b = num;
			img->data[i*img->width + j].g = num;
			img->data[i*img->width + j].r = num;

			free(matrix);
		}
	}

	free(tmpdata);
}

int _tmain(int argc, _TCHAR* argv[])
{
	IMAGE *img;
    loadBMP("lena.bmp", &img);

    //average_greyscale(img);
	//writeBMP("average_greyscale.bmp", img);

	//binary(img,100);
	//writeBMP("binary.bmp", img);

	adaptivebinary(img);
	writeBMP("adaptivebinary.bmp", img);

	//gussianblur(img);
	//writeBMP("gussianblur.bmp", img);

	//median(img,5,5);
	//writeBMP("median.bmp", img);

    freeBMP(img);
    return 0;

};