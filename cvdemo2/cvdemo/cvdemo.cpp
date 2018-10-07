// cvdemo.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include "loadBMP.h"

void average_greyscale(IMAGE *img)
{
    printf("Converting to greyscale\n");
    for(uint32_t i = 0; i < img->width*img->height; i++ ) 
	{
        uint8_t avg = (img->data[i].r + img->data[i].g + img->data[i].b)/3;
        img->data[i].r = avg;
        img->data[i].g = avg;
        img->data[i].b = avg;
    }
}

void guassian(IMAGE* img)
{
	ColorRGB* tempdata = (ColorRGB*)malloc(img->height * img->width * sizeof(ColorRGB));
	memcpy(tempdata,img->data,img->height * img->width * sizeof(ColorRGB));

	int mask[9]  =   {1,2,1,  //1/16
					  2,4,2,
					  1,2,1};

	for(int i=1;i<img->height-1;i++)
	{
		for(int j=1;j<img->width-1;j++)
		{
			int index=0,weights=0;
			int sum_b=0,sum_g=0,sum_r=0;
			for(int m=i-1;m<i+2;m++)
			{
				for(int n=j-1;n<j+2;n++)
				{
					index = m * img->width + n;
					sum_b +=tempdata[index].b * mask[weights];
					sum_g +=tempdata[index].g * mask[weights];
					sum_r +=tempdata[index].r * mask[weights];
					weights++;
				}
			}
			img->data[i*img->width+j].b = sum_b/16;
			img->data[i*img->width+j].g = sum_g/16;
			img->data[i*img->width+j].r = sum_r/16;
		}
	}


}

void binary(IMAGE *img,int thr ) //固定阈值二值化
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

void dilate(IMAGE *img) //膨胀(计算核覆盖区域的最大值赋值给参考点)
{
	//printf("dilate\n");
	//拷贝图像数据
	uint32_t sum = img->height * img->width*sizeof(ColorRGB);
	ColorRGB *tempdata =  (ColorRGB*) malloc(sum);
	memcpy(tempdata,img->data,sum);

	for(uint32_t i =1;i<img->height-1;i++)
	{
		for(uint32_t j = 1;j<img->width-1;j++)
		{
			uint32_t max = 0 , avg = 0;//(i,j)所在核的最大值
			for(uint32_t m=i-1;m<i+2;m++)
			{
				for(uint32_t n=j-1;n<j+2;n++)
				{
					avg = (tempdata[m*img->width+n].r + tempdata[m*img->width+n].g + tempdata[m*img->width+n].b)/3;
					if(avg>max)
						max = avg;
				
				}
			}
			img->data[i*img->width+j].r=img->data[i*img->width+j].g=img->data[i*img->width+j].b = max;
		}
	}

	free(tempdata);

}

void erode(IMAGE* img)//腐蚀
{
	//拷贝图像数据
	uint32_t sum = img->height * img->width * sizeof(ColorRGB);
	ColorRGB* tempdata =(ColorRGB*) malloc(sum*sizeof(ColorRGB));
	memcpy(tempdata,img->data,sum);

	for(int i=1;i<img->height-1;i++)
	{
		for(int j=1;j<img->width-1;j++)
		{
			uint32_t min=256 ,avg=0;
			for(int m=i-1;m<i+2;m++)
			{
				for(int n=j-1;n<j+2;n++)
				{
					avg = (tempdata[m*img->width+n].r+ tempdata[m*img->width+n].g+ tempdata[m*img->width+n].b)/3;
					if(avg<min)
					min = avg;
				}
			}
			img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = min;
		}
	}
	free(tempdata);
}

void open(IMAGE* img)//先腐蚀后膨胀，消除小物体，纤细处分离物体
{
	//binary(img,100);
	erode(img);
	dilate(img);
	
}

void close(IMAGE* img)//先膨胀后腐蚀，排除小型黑洞，往往图像边缘粗糙
{
	//binary(img,100);
	dilate(img);
	erode(img);
}

void morph_grad(IMAGE* img)//形态学梯度(膨胀与腐蚀的差)
{
	uint32_t num = img->height*img->width*sizeof(ColorRGB);
	ColorRGB* tempdata = (ColorRGB*)malloc(num);
	memcpy(tempdata ,img->data , num);

	for(int i =1;i<img->height-1;i++)
	{
		for(int j=1;j<img->width-1;j++)
		{
			int min=255,max=0,avg=0;
			for(int m=i-1;m<i+2;m++)
			{
				for(int n=j-1;n<j+2;n++)
				{
					avg = (tempdata[m*img->width+n].r+tempdata[m*img->width+n].g+tempdata[m*img->width+n].b)/3;
					if(avg<min)   min=avg;
					if(avg>max)   max=avg;
				}
						
			}
			img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = max-min;

		}
	}
	free(tempdata);

}

void top_hat(IMAGE* img)//原图-开运算，分离比周围亮的斑块
{
	uint32_t sum = img->height * img->width * sizeof(ColorRGB);
	ColorRGB* tempdata = (ColorRGB*)malloc(sum);
	memcpy(tempdata,img->data,sum);//原图像数据

	open(img);
	ColorRGB* opendata = (ColorRGB*)malloc(sum);
	memcpy(opendata,img->data,sum);//开运算后的数据
	int index = 0,avg1 =0,avg2 =0;
	for(int i= 1;i<img->height-1;i++)
	{
		for (int j=1;j<img->width-1;j++)
		{
			index =i * img->width + j ;
			avg1 = (tempdata[index].r + tempdata[index].g + tempdata[index].b)/3;
			avg2 = (opendata[index].r + opendata[index].g + opendata[index].b)/3;
			img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = avg1 - avg2;
		
		}
	}



}

void black_hat(IMAGE* img)//闭运算-原图，突出比周围更暗区域
{
	uint32_t sum = img->height * img->width * sizeof(ColorRGB);
	ColorRGB* tempdata = (ColorRGB*)malloc(sum);
	memcpy(tempdata,img->data,sum);//原图像数据

	close(img);
	ColorRGB* closedata = (ColorRGB*)malloc(sum);
	memcpy(closedata ,img->data,sum);//闭运算后的数据

	int index = 0,avg1=0,avg2=0;
	for(int i=1;i<img->height;i++)
	{
		for(int j=1;j<img->width;j++)
		{
			index = i*img->width + j;
			avg1 = (closedata[index].r+closedata[index].g+closedata[index].b)/3;
			avg2 = (tempdata[index].r+tempdata[index].g+tempdata[index].b)/3;
			img->data[index].r = img->data[index].g = img->data[index].b = avg1 - avg2;		}
	
	}

}

void pyrDown(IMAGE* img)
{
	guassian(img);

	IMAGE* pd;
    loadBMP("lena2.bmp", &pd);
	for(int i =0;i< pd->height;i++)
	{
		for(int j=0;j<pd->width;j++)
		{
			pd->data[i*pd->width+j].r = img->data[2*i*img->width + 2*j].r;
			pd->data[i*pd->width+j].g = img->data[2*i*img->width + 2*j].g;
			pd->data[i*pd->width+j].b = img->data[2*i*img->width + 2*j].b;
		}
	}

	
	writeBMP("pyrDown.bmp", pd);				
	freeBMP(pd);

}

void pyrUp()
{
	IMAGE *img;
    loadBMP("pyrDown.bmp",&img);
	IMAGE *pu;
	loadBMP("lena.bmp",&pu);

	for(int i=0;i<pu->height;i++)
	{
		for(int j=0;j<pu->width;j++)
		{
			if(i%2 == 0 || j%2 == 0)
			{	
				pu->data[i*pu->width+j].r = 0;
				pu->data[i*pu->width+j].g = 0;
				pu->data[i*pu->width+j].b = 0;
			}else
			{
				pu->data[i*pu->width+j].r = img->data[((i-1)*img->width+j-1)/2].r;
				pu->data[i*pu->width+j].g = img->data[((i-1)*img->width+j-1)/2].g;
				pu->data[i*pu->width+j].b = img->data[((i-1)*img->width+j-1)/2].b;
			}

		}
	}
	guassian(pu);

	writeBMP("pyrUp.bmp",pu);
	freeBMP(img);
	freeBMP(pu);

}

void roberts(IMAGE* img)
{
	guassian(img);
	average_greyscale(img);

	uint32_t sum = img->height * img->width * sizeof(ColorRGB);
	ColorRGB* tempdata =(ColorRGB*) malloc(sum*sizeof(ColorRGB));
	memcpy(tempdata,img->data,sum);
	     //  [1,0        [0 ,1
	     //   0,-1]       -1,0]
	for(int i=0;i<img->height-1;i++)
	{
		for(int j=0;j<img->width-1;j++)
		{
			int Gx = tempdata[i*img->width+j].r - tempdata[(i+1)*img->width+(j+1)].r;
			int Gy = tempdata[i*img->width+j+1].r - tempdata[(i+1)*img->width+j].r;
			int G = abs(Gx)+abs(Gy);
			if(G>80) {img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = 255;}
			else {img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = 0;}
		}
	}

}

void sobel(IMAGE* img)
{
	average_greyscale(img);
	guassian(img);

	uint32_t sum = img->height * img->width * sizeof(ColorRGB);
	ColorRGB* tempdata =(ColorRGB*) malloc(sum*sizeof(ColorRGB));
	memcpy(tempdata,img->data,sum);
	     //  [-1,0,+1        [-1,-2,-1
	     //   -2,0,+2          0, 0, 0  
	     //   -1,0,+1 ]       +1,+2,+1]
	int coGx[9] = {-1,0,+1,-2,0,+2,-1,0,+1};
	int coGy[9] = {-1,-2,-1,0,0,0,+1,+2,+1};

	for(int i=1;i<img->height-1;i++)
	{
		for(int j=1;j<img->width-1;j++)
		{
			int index = 0,k = 0,Gx = 0,Gy = 0;
			for(int m =i-1;m<i+2;m++)
			{
				for(int n=j-1;n<j+2;n++)
				{
					index = m*img->width+n;
					Gx += coGx[k] * tempdata[index].r;
					Gy += coGy[k] * tempdata[index].r;	
					k++;
				}
			}
			int G = abs(Gx)+abs(Gy);
			//img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = G;
			if(G>100) {img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = 255;}
			else {img->data[i*img->width+j].r = img->data[i*img->width+j].g = img->data[i*img->width+j].b = 0;}
		}
	}


}

void Laplacian(IMAGE* img)
{
	guassian(img);
	average_greyscale(img);
	
	ColorRGB* tempdata = (ColorRGB*)malloc(img->height * img->width * sizeof(ColorRGB));
	memcpy(tempdata,img->data,img->height * img->width * sizeof(ColorRGB));

	int mask[9]  =   {0,-1,0,  //1/16
					  -1,4,-1,
					  0,-1,0};

	for(int i=1;i<img->height-1;i++)
	{
		for(int j=1;j<img->width-1;j++)
		{
			int index=0,weights=0,sum=0;
			for(int m=i-1;m<i+2;m++)
			{
				for(int n=j-1;n<j+2;n++)
				{
					index = m * img->width + n;
					sum +=tempdata[index].r * mask[weights];
					weights++;
				}
			}
			img->data[i*img->width+j].b = img->data[i*img->width+j].g = img->data[i*img->width+j].r = abs(sum);
			
		}
	}


}
	
void hist(IMAGE* img)
{
	average_greyscale(img);
	int histogram[256]={0};
	for(int i=0;i<img->width * img->height;i++)
	{
		for(int j=0;j<256;j++)
		{
			if(img->data[i].r == j)
				histogram[j]++;
		}
	
	}
	for(int i=0;i<256;i++)
	{
		printf("%d  ",histogram[i]);
		if((i+1)/16==0) printf("\n");
	}
	system("pause");
	

}

int _tmain(int argc, _TCHAR* argv[])
{
	IMAGE *img;
    loadBMP("lena.bmp", &img);
	getchar();
    average_greyscale(img);
	//binary(img,100);
	//dilate(img);
	//erode(img);
	//open(img);
	//close(img);
	//morph_grad(img);
    //top_hat(img);
	//black_hat(img);
	//pyrUp();
	//pyrDown(img);
	//roberts(img);
	//sobel(img);
	//Laplacian(img);
	//hist(img);
	writeBMP("hi.bmp", img);

	getchar();
    freeBMP(img);
    return 0;

}