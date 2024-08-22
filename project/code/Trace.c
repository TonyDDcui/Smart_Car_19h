#include "Trace.h"

/*tft180_x_max = 160;
tft180_y_max = 128;
MT9V03X_W = (160)       
MT9V03X_H = (128)*/
uint8 search_line_end = 10;     //搜线终止行
uint8 image_yuanshi[MT9V03X_H][MT9V03X_W];//储存摄像头的图像数组
uint8 image_01[MT9V03X_H][MT9V03X_W];

extern const uint8 Image_Flags[][9][8];     //放在图上的数字标记
uint8 image_two_value[MT9V03X_H][MT9V03X_W];//二值化后的原数组
volatile int Left_Line[MT9V03X_H]; //左边线数组
volatile int Right_Line[MT9V03X_H];//右边线数组
volatile int Mid_Line[MT9V03X_H];  //中线数组
volatile int Road_Wide[MT9V03X_H]; //赛宽数组
volatile int White_Column[MT9V03X_W];    //每列白列长度
volatile int Search_Stop_Line;     //搜索截止行,只记录长度，想要坐标需要用视野高度减去该值
volatile int Boundry_Start_Left;   //左右边界起始点
volatile int Boundry_Start_Right;  //第一个非丢线点,常规边界起始点
volatile int Left_Lost_Time;       //边界丢线数
volatile int Right_Lost_Time;
volatile int Both_Lost_Time;//两边同时丢线数
//十字
volatile int Cross_Flag=0;
volatile int Left_Down_Find=0; //十字使用，找到被置行数，没找到就是0
volatile int Left_Up_Find=0;   //四个拐点标志
volatile int Right_Down_Find=0;
volatile int Right_Up_Find=0;
volatile int Island_State=0;     //环岛状态标志
volatile int Ramp_Flag=0;//坡道标志
volatile float Err=0;     //摄像头误差
volatile int Zebra_Stripes_Flag=0;

int Longest_White_Column_Left[2]; //最长白列,[0]是最长白列的长度，也就是Search_Stop_Line搜索截止行，[1】是第某列
int Longest_White_Column_Right[2];//最长白列,[0]是最长白列的长度，也就是Search_Stop_Line搜索截止行，[1】是第某列
int Left_Lost_Flag[MT9V03X_H] ; //左丢线数组，丢线置1，没丢线置0
int Right_Lost_Flag[MT9V03X_H]; //右丢线数组，丢线置1，没丢线置0

void Zebra_Stripes_Detect(void);
void Cross_Detect(void);

const uint8 Standard_Road_Wide[MT9V03X_H]=//标准赛宽
{ 10, 12, 14, 16, 18, 20, 22, 24, 26, 28,
  30, 32, 34, 36, 38, 40, 42, 44, 46, 48,
  50, 52, 54, 56, 58, 60, 62, 64, 66, 68,
  70, 72, 74, 76, 78, 80, 82, 84, 86, 88,
  90, 92, 94, 96, 98,100,102,104,106,108,
 110,112,114,116,118,120,122,124,126,128,
 130,132,134,136,138,140,142,144,146,148};

struct ROAD_TYPE road_type = 
{
        .straight      = 0,//直道
        .bend          = 0,//弯道
        .Ramp          = 0,//坡道
        .Cross         = 0,//十字
        .L_Cross       = 0,
        .R_Cross       = 0,
        .LeftCirque    = 0,//左圆环
        .RightCirque   = 0,
        .Fork          = 0,//岔路

};


struct STATUS_TYPE status_type = 
{
        .start_pick    = 0,//开始捡卡片
        .stop_pick     = 0,
        .start_lay_W   = 0,//开始放置字母小类卡片
        .stop_lay_W    = 0,
        .start_lay_F   = 0,//开始放置数字大类卡片
        .stop_lay_F    = 0,
};

const uint8 Weight[MT9V03X_H]= //已将MT9V03X_H设置为90
{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端00 ——09 行权重
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端10 ——19 行权重
        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //图像最远端20 ——29 行权重
        6, 7, 9,11,13,15,17,19,20,20,              //图像最远端30 ——39 行权重
       19,17,15,13,11, 9, 7, 5, 3, 1,              //图像最远端40 ——49 行权重
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端50 ——59 行权重
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //图像最远端60 ——69 行权重

};


void image_ceshi(void)
{
    Transfer_Camera();
    Get01change_Dajin();
    Longest_White_Column();
    //Zebra_Stripes_Detect();
    //Cross_Detect();
    Err = Err_Sum();  
    //tft180_show_string(0,130,"Err=");
    //tft180_show_int(0,100,(int32)Err,3);
  
}


/*-------------------------------------------------------------------------------------------------------------------
  @brief     比较大小
---------------------
-------------------------------------------------------------------------------------------------------------------*/

int my_min(int a, int b) 
{
    return a < b ? a : b;
}

int my_max(int a, int b) 
{
    return a > b ? a : b;
}



//-------------------------------------------------------------------------------------------------------------------
//  存储到一个新数组，后续处理（High为120，Width为188，刷新率为50），不知道刷新率为什么50
//  @param      image  图像数组
//  @param      clo    宽
//  @param      row    高
//  @param      pixel_threshold 阈值分离
//-------------------------------------------------------------------------------------------------------------------
void Transfer_Camera(void)
{
    for(uint8 y=0; y<MT9V03X_H; y++)       
    {
        for(uint8 x=0; x<MT9V03X_W; x++)
        {
            image_yuanshi[y][x] = mt9v03x_image[y][x];
        }
    }
}


//-------------------------------------------------------------------------------------------------------------------
//  求二值化阈值
//  @param      image  图像数组
//  @param      clo    宽
//  @param      row    高
//  @param      pixel_threshold 阈值分离
//-------------------------------------------------------------------------------------------------------------------
uint8 Threshold_Deal(uint8* image, uint16 col, uint16 row, uint32 pixel_threshold)
{
#define GrayScale 256//灰度级数GrayScale为256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];//像素计数，数组下标表示灰度级，数组值代表个数
    float pixelPro[GrayScale];
    int i, j;
    int pixelSum = width * height;//像素和
    uint8 threshold = 0;
    uint8* data = image;  //指向像素数据的指针
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum = 0;
    //统计灰度级中每个像素在整幅图像中的个数
    for (i = 0; i < height; i += 1)
    {
        for (j = 0; j < width; j += 1)
        {
            // if((sun_mode&&data[i*width+j]<pixel_threshold)||(!sun_mode))
            //{假设图像的宽度为width，高度为height，
            //那么第i行第j列的像素在一维数组中的索引可以通过以下公式计算得到：
            //index=i×width+j
          //data[i]数组的值为灰度级
            pixelCount[(int)data[i * width + j]]++;  //将当前点的灰度级中像素值作为计数数组的下标
            gray_sum += (int)data[i * width + j];  //灰度值总和
            //}
        }
    }

    //计算每个像素值的点在整幅图像中的比例
    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //遍历灰度级[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (j = 0; j < pixel_threshold; j++)// pixel_threshold阈值分离，可以自己设置
    {
       //pixelPro[j]像素值的点在整幅图像中的比例
        //背景部分的权重和临时值
        w0 += pixelPro[j];  //背景部分每个灰度值的像素点所占比例之和 即背景部分的比例，应该是权重
        u0tmp += j * pixelPro[j];  //背景部分 每个灰度值的点的比例 *灰度值
        
        //前景部分的权重和临时值
        w1 = 1 - w0;
        u1tmp = gray_sum / pixelSum - u0tmp;

        //平均值除以权重
        u0 = u0tmp / w0;    //背景平均灰度
        u1 = u1tmp / w1;    //前景平均灰度
        u = u0tmp + u1tmp;  //全局平均灰度
        
        /*这部分计算了背景部分的平均灰度与全局平均灰度之间的差异的平方（数字2），
        并乘以背景部分的权重。这个值越大表示背景部分与全局平均灰度之间的差异越大，
        也就是说明当前阈值对于背景部分的分割效果越好。*/
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);//这个值越大表示当前阈值对于整体图像的分割效果越好
        if (deltaTmp > deltaMax)//更新二值阈值
        {
            deltaMax = deltaTmp;
            threshold = (uint8)j;     //本来这里没有强制类型转换的,我自己加的
        }
        if (deltaTmp < deltaMax)
        {
            break;
        }
    }
    return threshold;
}


//-------------------------------------------------------------------------------------------------------------------
//  二值化
//  将二值化图像数组转存至image_01[y][x]
// 滤波
//-------------------------------------------------------------------------------------------------------------------
uint8 Threshold;  //阈值
uint8 Threshold_static = 150;   //阈值静态下限160 220  230            白天：200   晚上：230  A:200
uint16 Threshold_detach = 100;  //阳光算法分割阈值(光强越强,该值越大)   白天：300
void Get01change_Dajin(void)//将输入的图像进行二值化
{               //用于计算图像的阈值
    Threshold = Threshold_Deal(image_yuanshi[0], MT9V03X_W, MT9V03X_H, Threshold_detach);

    if (Threshold < Threshold_static)
    {
        Threshold = Threshold_static;
    }

    uint8 thre;
    for(uint8 y = 0; y < MT9V03X_H; y++)
    {
        for(uint8 x = 0; x < MT9V03X_W; x++)
        {
            if (x <= 1)
                thre = Threshold - 10;
            else if (x >= MT9V03X_W-1)
                thre = Threshold - 10;
            else
                thre = Threshold;

            if (image_yuanshi[y][x] >thre)         //数值越大，显示的内容越多，较浅的图像也能显示出来
                image_01[y][x] = 255;  //白
            else
                image_01[y][x] = 0;  //黑  二值化完毕
        }
    }
     Pixle_Filter();//二值化之后进行滤波
}


//-------------------------------------------------------------------------------------------------------------------
//像素滤波,去除偶尔出现的噪点,效果有限，
//-------------------------------------------------------------------------------------------------------------------
void Pixle_Filter(void)
{

    for (uint8 height = 10; height < MT9V03X_H-10; height++)
    {
        for (uint8 width = 10; width < MT9V03X_W -10; width = width + 1)
        {
            if ((image_01[height][width] == 0) && (image_01[height - 1][width] + image_01[height + 1][width] +image_01[height][width + 1] + image_01[height][width - 1] >=3*255))
            { //一个黑点的上下左右的白点大于等于三个，令这个点为白
                image_01[height][width] = 255;//将1改为了255
            }//一个白点的上下左右的黑点大于等于三个，令这个点为黑                                        
            else if((image_01[height][width] == 255)&&(image_01[height-1][width]+image_01[height+1][width]+image_01[height][width+1]+image_01[height][width-1]<=255))
            {
                image_01[height][width] =0;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     双最长白列巡线
  @param     null
  @return    null
  Sample     Longest_White_Column_Left();
  @note      最长白列巡线，寻找初始边界，丢线，最长白列等基础元素，后续读取这些变量来进行赛道识别
-------------------------------------------------------------------------------------------------------------------*/
void Longest_White_Column()//最长白列巡线
{
    int i, j;
    int start_column=20;//最长白列的搜索开始列，第二十列开始搜
    int end_column=MT9V03X_W-20;//最长白列的搜索截至列，第W-20截至搜
    int left_border = 0, right_border = 0;//临时存储赛道位置
    Longest_White_Column_Left[0] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
    Longest_White_Column_Left[1] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
    Longest_White_Column_Right[0] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
    Longest_White_Column_Right[1] = 0;//最长白列,[0]是最长白列的长度，[1】是第某列
    Right_Lost_Time = 0;    //边界丢线数
    Left_Lost_Time  = 0;
    Boundry_Start_Left  = 0;//第一个非丢线点,常规边界起始点
    Boundry_Start_Right = 0;
    Both_Lost_Time = 0;//两边同时丢线数
    //tft180_draw_line ( MT9V03X_W/2, 0, MT9V03X_W/2, MT9V03X_H-1, RGB565_RED);

    for (i = 0; i <=MT9V03X_H-1; i++)//数据清零
    {
        Right_Lost_Flag[i] = 0;
        Left_Lost_Flag[i] = 0;
        Left_Line[i] = 0;//左边线数组
        Right_Line[i] = MT9V03X_W-1;
    }
    for(i=0;i<=MT9V03X_W-1;i++)//每列白列长度清0
    {
        White_Column[i] = 0;
    }

   //环岛需要对最长白列范围进行限定
    /*//环岛3状态需要改变最长白列寻找范围
    if(Right_Island_Flag==1)//右环
    {
        if(Island_State==3)
        {
            start_column=40;//最长白列的搜索区间
            end_column=MT9V03X_W-20;
        }
    }
    else if(Left_Island_Flag==1)//左环
    {
        if(Island_State==3)
        {
            start_column=20;
            end_column=MT9V03X_W-40;
        }
    }*/
    //从左到右，从下往上，遍历全图记录范围内的每一列白点数量
    
    for (j =start_column; j<=end_column; j++)
    {
        for (i = MT9V03X_H-1; i >= 10; i--)
        {
            if(image_01[i][j] == IMG_BLACK)
                break;//跳出内部for
            else
                White_Column[j]++;//数组位数代表哪一列，数组的值代表对应列的白点数量
        }
    }

    //从左到右找左边最长白列
    Longest_White_Column_Left[0] =0;/*最长白列,第零位表示最长白列的长度，第一位是最长列的列数
    也就是Search_Stop_Line搜索截止行，[1】是第某列*/
    for(i=start_column;i<=end_column;i++)
    {
        if (Longest_White_Column_Left[0] < White_Column[i])//找最长的那一列
        {
            Longest_White_Column_Left[0] = White_Column[i];//【0】是白列长度，将当前列的白列长度赋值到
                                                             //Longest_White_Column_Left的第0位
            Longest_White_Column_Left[1] = i;              //【1】是下标，第j列
        }
    }
    //从右到左找右边最长白列
    Longest_White_Column_Right[0] = 0;//【0】是白列长度
    for(i=end_column;i>=start_column;i--)//从右往左，注意条件，找到左边最长白列位置就可以停了
    {
        if (Longest_White_Column_Right[0] < White_Column[i])//找最长的那一列
        {
            Longest_White_Column_Right[0] = White_Column[i];//【0】是白列长度
            Longest_White_Column_Right[1] = i;              //【1】是下标，第j列
        }
    }

//搜索截至行定为赛道当前最长白列的那列长度，人为应该可以进行缩短，毕竟每个赛题的任务不同
//对于直道就是图像最高，对于弯道和特殊元素需测试
    Search_Stop_Line = Longest_White_Column_Left[0];//搜索截止行选取左或者右区别不大，他们两个理论上是一样的
    //画出截至行
    //tft180_draw_line ( 0, Search_Stop_Line, MT9V03X_W-1, Search_Stop_Line, RGB565_RED);
    //在屏幕理论中线处显示，用于调整摄像头
    
    
    for (i = MT9V03X_H - 1; i >=MT9V03X_H-Search_Stop_Line; i--)//常规巡线
    {//               表示最长白列的列数，很牛，向右扫列
        for (j = Longest_White_Column_Right[1]; j <= MT9V03X_W - 1 ; j++)
        {     
            if (image_01[i][j] ==IMG_WHITE && image_01[i][j + 1] == IMG_BLACK && image_01[i][j + 2] == IMG_BLACK)//白黑黑，找到右边界
            {
                right_border = j;//储存赛道位置，值表示第几列，
                Right_Lost_Flag[i] = 0; //右丢线数组，表示第i行丢不丢 丢线置1，不丢线置0
                break;
            }
            else if(j>=MT9V03X_W-1)//没找到右边界，把屏幕最右赋值给右边界
            {
                right_border = j;
                Right_Lost_Flag[i] = 1; //右丢线数组，丢线置1，不丢线置0
                break;
            }
        }
        for (j = Longest_White_Column_Left[1]; j >= 0; j--)//往左边扫描
        {
            if (image_01[i][j] ==IMG_WHITE && image_01[i][j - 1] == IMG_BLACK && image_01[i][j - 2] == IMG_BLACK)//黑黑白认为到达左边界
            {
                left_border = j;
                Left_Lost_Flag[i] = 0; //左丢线数组，丢线置1，不丢线置0
                break;
            }
            else if(j<=0)
            {
                left_border = j;//找到头都没找到边，就把屏幕最左右当做边界
                Left_Lost_Flag[i] = 1; //左丢线数组，丢线置1，不丢线置0
                break;
            }
        }
        
        //i表示行，值代表列
        Left_Line [i] = left_border;       //左边线线数组，整个值表示当前边界的第几列
        Right_Line[i] = right_border;      //右边线线数组//可以考虑在此画边界
        Mid_Line[i] = ( Left_Line[i] +Right_Line[i])/2; 
        
        //tft180_draw_point(left_border,i,RGB565_BLUE);//左边线
        //tft180_draw_point(right_border,i,RGB565_BLUE);//右边线
        //tft180_draw_point(Mid_Line[i],i,RGB565_PURPLE);
        //offset +=offset_quanzhong[MT9V03X_H-30 -i] *(Mid_Line[i] -MT9V03X_W/2);
    }
      
    for (i = MT9V03X_H - 1; i >= 0; i--)//赛道数据初步分析
    {
        if (Left_Lost_Flag[i]  == 1)//单边丢线数
            Left_Lost_Time++;
        if (Right_Lost_Flag[i] == 1)
            Right_Lost_Time++;
        if (Left_Lost_Flag[i] == 1 && Right_Lost_Flag[i] == 1)//双边丢线数
            Both_Lost_Time++;
        //Boundry_Start_Left 左右边界起始点，只记录一次
        if (Boundry_Start_Left ==  0 && Left_Lost_Flag[i]  != 1)//记录第一个非丢线点，边界起始点
            Boundry_Start_Left = i;
        if (Boundry_Start_Right == 0 && Right_Lost_Flag[i] != 1)
            Boundry_Start_Right = i;
        Road_Wide[i]=Right_Line[i]-Left_Line[i];//路的宽度
        
//        tft180_show_string(0,100,"Road_Wide=");
//        tft180_show_int(80,100,(int32)Road_Wide[i],4);
//        tft180_show_string(0,130,"BothLostTime=");
//        tft180_show_int(95,130,(int32) Both_Lost_Time,3);
        
    }
    
    
    
    
    //环岛3状态改变边界，看情况而定，我认为理论上的最优情况是不需要这些处理的
    /*f(Island_State==3||Island_State==4)
    {
        if(Right_Island_Flag==1)//右环
        {
            for (i = MT9V03X_H - 1; i >= 0; i--)//右边直接写在边上
            {
                Right_Line[i]=MT9V03X_W-1;
            }
        }
        else if(Left_Island_Flag==1)//左环
        {
            for (i = MT9V03X_H - 1; i >= 0; i--)//左边直接写在边上
            {
                Left_Line[i]=0;      //右边线线数组
            }
        }
    }*/
}



/*-------------------------------------------------------------------------------------------------------------------
  @brief     左补线
  @param     补线的起点，终点
  @return    null
  Sample     Left_Add_Line(int x1,int y1,int x2,int y2);
  @note      补的直接是边界，点最好是可信度高的,不要乱补
先暂时用吧，暂时不懂
-------------------------------------------------------------------------------------------------------------------*/
void Left_Add_Line(int x1,int y1,int x2,int y2) //左补线,补的是边界
{
    int i,a1,a2;
    int hx;
//若坐标大于图像宽则坐标为图像宽，若坐标小于则为0
    x1 = my_max(0, my_min(x1, MT9V03X_W-1));
    y1 = my_max(0, my_min(y1, MT9V03X_H-1));
    x2 = my_max(0, my_min(x2, MT9V03X_W-1));
    y2 = my_max(0, my_min(y2, MT9V03X_H-1));

    a1=y1;
    a2=y2;
    
    //if(a1>a2)//坐标互换{ max=a1;a1=a2;a2=max;}
    if(a1 > a2) 
    { a1 ^= a2; a2 ^= a1; a1 ^= a2; }
    
    for(i=a1;i<=a2;i++)//根据斜率补线即可
    {
        hx=(i-y1)*(x2-x1)/(y2-y1)+x1;
        //if(hx>=MT9V03X_W)hx=MT9V03X_W;else if(hx<=0)hx=0;
        hx = my_max(0, my_min(hx, MT9V03X_W));
        Left_Line[i]=hx; //i表示行，值代表列
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     右补线
  @param     补线的起点，终点
  @return    null
  Sample     Right_Add_Line(int x1,int y1,int x2,int y2);
  @note      补的直接是边界，点最好是可信度高的，不要乱补
-------------------------------------------------------------------------------------------------------------------*/
void Right_Add_Line(int x1,int y1,int x2,int y2)//右补线,补的是边界
{
    int i,a1,a2;
    int hx;

    x1 = my_max(0, my_min(x1, MT9V03X_W-1));
    y1 = my_max(0, my_min(y1, MT9V03X_H-1));
    x2 = my_max(0, my_min(x2, MT9V03X_W-1));
    y2 = my_max(0, my_min(y2, MT9V03X_H-1));
    
    a1=y1;
    a2=y2;
    //if(a1>a2)//坐标互换{max=a1;a1=a2;a2=max;}
    if(a1 > a2) 
    {
    a1 ^= a2;
    a2 ^= a1;
    a1 ^= a2;
    }
    for(i=a1;i<=a2;i++)//根据斜率补线即可
    {
        hx=(i-y1)*(x2-x1)/(y2-y1)+x1;
        //if(hx>=MT9V03X_W)hx=MT9V03X_W;else if(hx<=0)hx=0; 
        hx = my_max(0, my_min(hx, MT9V03X_W)); 
        Right_Line[i]=hx;
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     找下面的两个拐点的行数，使用之后，
             Left_down_Find
             Right_down_Find两个的值代表上面左右拐点在的行数
             供十字使用
  @param     搜索的范围起点，终点
  @return    修改两个全局变量
             Right_Down_Find=0;
             Left_Down_Find=0;
  Sample     Find_Down_Point(int start,int end)
  @note      运行完之后查看对应的变量，注意，没找到时对应变量将是0
-------------------------------------------------------------------------------------------------------------------*/
void Find_Down_Point(int start,int end)
{
    
    Right_Down_Find=0;
    Left_Down_Find=0;
    /*if(start<end)
    {
        t=start;
        start=end;
        end=t;
    }*/
    if(start<end) 
    {
    start ^= end;
    end ^= start;
    start ^= end;
    }
    /*if(start>=MT9V03X_H-1-5)//下面5行数据不稳定，不能作为边界点来判断，舍弃
        start=MT9V03X_H-1-5;*///具体舍不舍弃得根据实验
      
    /*if(end<=MT9V03X_H-Search_Stop_Line)
        end=MT9V03X_H-Search_Stop_Line;
    if(end<=5)
       end=5;*/
    start = my_min(MT9V03X_H - 1 - 5, start);
    if(end <= MT9V03X_H - Search_Stop_Line || end <= 5)
        {end = my_max(MT9V03X_H - Search_Stop_Line, 5);}
    
    for(int i=start;i>=end;i--)
    {
        if(Left_Down_Find==0&&//只找第一个符合条件的点
                 //i表示行，值代表列
           abs(Left_Line[i]-Left_Line[i+1])<=5&&//角点的阈值可以更改
           abs(Left_Line[i+1]-Left_Line[i+2])<=5&&
           abs(Left_Line[i+2]-Left_Line[i+3])<=5&&
              (Left_Line[i]-Left_Line[i-2])>=8&&
              (Left_Line[i]-Left_Line[i-3])>=15&&
              (Left_Line[i]-Left_Line[i-4])>=15)
        {
            Left_Down_Find=i;//获取行数即可
        }
        if(Right_Down_Find==0&&//只找第一个符合条件的点
           abs(Right_Line[i]-Right_Line[i+1])<=5&&//角点的阈值可以更改
           abs(Right_Line[i+1]-Right_Line[i+2])<=5&&
           abs(Right_Line[i+2]-Right_Line[i+3])<=5&&
              (Right_Line[i]-Right_Line[i-2])<=-8&&
              (Right_Line[i]-Right_Line[i-3])<=-15&&
              (Right_Line[i]-Right_Line[i-4])<=-15)
        {
            Right_Down_Find=i;
        }
        if(Left_Down_Find!=0 && Right_Down_Find!=0)//两个找到就退出
        {
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     找上面的两个拐点的行数，使用之后，
             Left_Up_Find
             Right_Up_Find两个的值代表上面左右拐点在的行数
            供十字使用
  @param     搜索的范围起点，终点
  @return    修改两个全局变量
             Left_Up_Find=0;
             Right_Up_Find=0;
  Sample     Find_Up_Point(int start,int end)
  @note      运行完之后查看对应的变量，注意，没找到时对应变量将是0
-------------------------------------------------------------------------------------------------------------------*/
void Find_Up_Point(int start,int end)//输入的是坐标
{
    Left_Up_Find=0;//标志
    Right_Up_Find=0;
/* if(start<end){t=start;start=end;end=t;} */
   
    if(start<end) //交换值
    { start ^= end; end ^= start; start ^= end;}
    
   /* if(end<=MT9V03X_H-Search_Stop_Line)
        end=MT9V03X_H-Search_Stop_Line;
    if(end<=5)                        //及时最长白列非常长，也要舍弃部分点，防止数组越界
        end=5;
    if(start>=MT9V03X_H-1-5)              //下面5行数据不稳定，不能作为边界点来判断，舍弃
        start=MT9V03X_H-1-5;*/
    if(end <= MT9V03X_H - Search_Stop_Line || end <= 5)
        {end = my_max(MT9V03X_H - Search_Stop_Line, 5);}
    start = my_min(MT9V03X_H - 1 - 5, start);
    for(int i=start;i>=end;i--)
    {
        if(Left_Up_Find==0&&//只找第一个符合条件的点
            //i表示行，值代表列
           abs(Left_Line[i]-Left_Line[i-1])<=5&&//左边第i行-左边第i行的下三行之间列差值小于5
           abs(Left_Line[i-1]-Left_Line[i-2])<=5&&
           abs(Left_Line[i-2]-Left_Line[i-3])<=5&&
              (Left_Line[i]-Left_Line[i+2])>=8&&
              (Left_Line[i]-Left_Line[i+3])>=15&&
              (Left_Line[i]-Left_Line[i+4])>=15)
        {
            Left_Up_Find=i;//获取行数即可
        }
        if(Right_Up_Find==0&&//只找第一个符合条件的点
           abs(Right_Line[i]-Right_Line[i-1])<=5&&//下面两行位置差不多
           abs(Right_Line[i-1]-Right_Line[i-2])<=5&&
           abs(Right_Line[i-2]-Right_Line[i-3])<=5&&
              (Right_Line[i]-Right_Line[i+2])<=-8&&
              (Right_Line[i]-Right_Line[i+3])<=-15&&
              (Right_Line[i]-Right_Line[i+4])<=-15)
        {
            Right_Up_Find=i;//获取行数即可
        }
        if(Left_Up_Find!=0&&Right_Up_Find!=0)//下面两个找到就出去
        {
            break;
        }
    }
    if(abs(Right_Up_Find-Left_Up_Find)>=30)//纵向撕裂过大，视为误判
    {
        Right_Up_Find=0;
        Left_Up_Find=0;
    }
}
/*-------------------------------------------------------------------------------------------------------------------
所有功能函数写完再进行元素判断
---------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------
  @brief     左边界延长
  @param     延长起始行数，延长到某行
  @return    null
  Sample     Stop_Detect(void)
  @note      从起始点向上找5个点，算出斜率，向下延长，直至结束点
-------------------------------------------------------------------------------------------------------------------*/
void Lengthen_Left_Boundry(int start,int end)
{
    int i,t;
    float k=0;
    //起始点位置校正，排除数组越界的可能
    start = my_max(0, my_min(MT9V03X_H - 1, start));
    end = my_max(0, my_min(MT9V03X_H - 1, end));
    
    if(end<start)//坐标互换
    {
        t=end;
        end=start;
        start=t;
    }

    if(start<=5)//因为需要在开始点向上找3个点，对于起始点过于靠上，不能做延长，只能直接连线
    {
         Left_Add_Line(Left_Line[start],start,Left_Line[end],end);
    }

    else
    {
        k=(float)(Left_Line[start]-Left_Line[start-4])/5.0;//这里的k是1/斜率
        for(i=start;i<=end;i++)
        {
            Left_Line[i]=(int)((i-start)*k+Left_Line[start]);//(x=(y-y1)*k+x1),点斜式变形
            
            /*if(Left_Line[i]>=MT9V03X_W-1){Left_Line[i]=MT9V03X_W-1;}
            else if(Left_Line[i]<=0){Left_Line[i]=0;}*/
            Left_Line[i] = my_max(0, my_min(MT9V03X_W - 1, Left_Line[i]));
            
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     右左边界延长
  @param     延长起始行数，延长到某行
  @return    null
  Sample     Stop_Detect(void)
  @note      从起始点向上找3个点，算出斜率，向下延长，直至结束点
-------------------------------------------------------------------------------------------------------------------*/
void Lengthen_Right_Boundry(int start,int end)
{
    int i,t;
    float k=0;
   /* if(start>=MT9V03X_H-1)//起始点位置校正，排除数组越界的可能
start=MT9V03X_H-1;else if(start<=0)start=0;if(end>=MT9V03X_H-1)end=MT9V03X_H-1;else if(end<=0)end=0;*/
    start = my_max(0, my_min(MT9V03X_H - 1, start));
    end = my_max(0, my_min(MT9V03X_H - 1, end));
    if(end<start)//++访问，坐标互换
    {
        t=end;
        end=start;
        start=t;
    }

    if(start<=5)//因为需要在开始点向上找3个点，对于起始点过于靠上，不能做延长，只能直接连线
    {
        Right_Add_Line(Right_Line[start],start,Right_Line[end],end);
    }
    else
    {
        k=(float)(Right_Line[start]-Right_Line[start-4])/5.0;//这里的k是1/斜率
        for(i=start;i<=end;i++)
        {
            Right_Line[i]=(int)((i-start)*k+Right_Line[start]);//(x=(y-y1)*k+x1),点斜式变形
            if(Right_Line[i]>=MT9V03X_W-1)
            {
                Right_Line[i]=MT9V03X_W-1;
            }
            else if(Right_Line[i]<=0)
            {
                Right_Line[i]=0;
            }
        }
    }
}


/*-------------------------------------------------------------------------------------------------------------------
  @brief     斑马线检测
  @param     null
  @return    null
    需要根据摄像头的位置判断具体的数值
  Sample     Zebra_Stripes_Detect(void)
  @note      边界起始靠下，最长白列较长，赛道宽度过窄，且附近大量跳变
-------------------------------------------------------------------------------------------------------------------*/

void Zebra_Stripes_Detect(void)
{
    int i=0,j=0;
    int change_count=0;//跳变计数
    int start_line=0;
    int endl_ine=0;
    int narrow_road_count=0;
    if(Cross_Flag!=0||(1<=Ramp_Flag&&Ramp_Flag<=3)
/*||Zebra_Stripes_Flag!=0||Stop_Flag!=0 ||Electromagnet_Flag!=0||Barricade_Flag*/!=0)//元素互斥，不是十字，不是，不是坡道，不是停车
    {
        return;
    }

    ////赛宽变化判斑马线
    if(Search_Stop_Line>=60&&//搜索截至行大于60，左最长白列大于等于30且小于等于W-30
       30<=Longest_White_Column_Left[1]&&Longest_White_Column_Left[1]<=MT9V03X_W-30&&
       30<=Longest_White_Column_Right[1]&&Longest_White_Column_Right[1]<=MT9V03X_W-30&&
         //左右边界起始点，代表行
       Boundry_Start_Left>=MT9V03X_H-15&&Boundry_Start_Right>=MT9V03X_H-15)
    {    //截止行长，最长白列的位置在中心附近，边界起始点靠下
        for(i=65;i>=20;i--)//在靠下的区域进行寻找赛道宽度过窄的地方
        {//      标准赛宽                赛道边界算出来的宽度
            if( (Standard_Road_Wide[i]-Road_Wide[i]) > 10 )//根据赛宽
            {
                narrow_road_count++;//多组赛宽变窄，才认为是斑马线
                if(narrow_road_count>=5)
                {
                    start_line=i;//记录赛道宽度很窄的位置 行数
                    break;//结束当前for循环
                }
            }
        }
    }
    if(start_line!=0)//多组赛宽变窄，以赛道过窄的位置为中心，划定一个范围，进行跳变计数
    {
        start_line=start_line+8;
        endl_ine=start_line-15;
        if(start_line>=MT9V03X_H-1)//限幅保护，防止数组越界
        {
            start_line=MT9V03X_H-1;
        }
        //插入mymax mymin
        if(endl_ine<=0)//限幅保护，防止数组越界
        {
            endl_ine=0;
        }
        for(i=start_line;i>=endl_ine;i--)//区域内跳变计数
        {
            for(j=Left_Line[i];j<=Right_Line[i];j++)
            {
                if(image_two_value[i][j+1]-image_two_value[i][j]!=0)
                {
                    change_count++;

                }
            }
        }
//        ips200_show_uint(0*16,100,change_count,5);//debug使用，查看跳变数，便于适应赛道
    }
 //画出区域，便于找bug，debug使用
//        Draw_Line( Left_Line[start_line], start_line, Left_Line[endl_ine], endl_ine);
//        Draw_Line( Left_Line[start_line], start_line, Right_Line[start_line], start_line);
//        Draw_Line(Right_Line[endl_ine], endl_ine, Right_Line[start_line], start_line);
//        Draw_Line(Right_Line[endl_ine], endl_ine, Left_Line[endl_ine], endl_ine);
//        ips200_draw_line ( Left_Line[start_line], start_line, Left_Line[endl_ine], endl_ine, RGB565_RED);
//        ips200_draw_line ( Left_Line[start_line], start_line, Right_Line[start_line], start_line, RGB565_RED);
//        ips200_draw_line (Right_Line[endl_ine], endl_ine, Right_Line[start_line], start_line, RGB565_RED);
//        ips200_draw_line (Right_Line[endl_ine], endl_ine, Left_Line[endl_ine], endl_ine, RGB565_RED);

    if(change_count>30)//跳变大于某一阈值，认为找到了斑马线
    {
        Zebra_Stripes_Flag=1;
    }
}


/*-------------------------------------------------------------------------------------------------------------------
  @brief     直道检测
  @param     null
  @return    null
  Sample     Straight_Detect()；
  @note      利用最长白列，边界起始点，中线起始点，
-------------------------------------------------------------------------------------------------------------------*/
void Straight_Detect(void)
{
    //Straight_Flag=0;先暂时注释
    if(Search_Stop_Line>=65)//截止行很远
    {
        if(Boundry_Start_Left>=68&&Boundry_Start_Right>=65)//起始点靠下
        {
            /*if(-5<=Err&&Err<=5)//误差很小
            {
                Straight_Flag=1;//认为是直道
            }*/
        }
    }
}






/*-------------------------------------------------------------------------------------------------------------------
  @brief     十字检测
  @param     null
  @return    null
  Sample     Cross_Detect(void);
  @note      利用四个拐点判别函数，查找四个角点，根据找到拐点的个数决定是否补线
-------------------------------------------------------------------------------------------------------------------*/
void Cross_Detect()
{
    int down_search_start=0;//下点搜索开始行
    Cross_Flag=0;
    if(Island_State==0 && Ramp_Flag==0)//与环岛，坡道互斥开，坡道可删了
    {
        Left_Up_Find=0; //四个拐点标志
        Right_Up_Find=0;
        if(Both_Lost_Time>=10)//丢线数，根据图像判断？，十字必定有双边丢线，在有双边丢线的情况下再开始找角点
        {
            Find_Up_Point( MT9V03X_H-1, 0 );//找上面的两个拐点，供十字使用，是不是范围太大了？
            //参一是起点，参二是终点
            if(Left_Up_Find==0 && Right_Up_Find==0)//只要没有同时找到两个上点，直接结束
            {
                return;//
            }
        }
        if(Left_Up_Find!=0 && Right_Up_Find!=0)//找到两个上点，就找到十字了
        {
            Cross_Flag=1;//对应标志位，便于各元素互斥掉
            
            //将Left_Up_Find和Right_Up_Find中较大的值赋给down_search_start
            //用两个上拐点坐标靠下者作为下点的搜索上限
            //down_search_start 下点搜索开始行
            down_search_start = Left_Up_Find>Right_Up_Find ? Left_Up_Find : Right_Up_Find;
            Find_Down_Point(MT9V03X_H-5,down_search_start+2);//在上拐点下2行作为下点的截止行
/*if(Left_Down_Find<=Left_Up_Find){Left_Down_Find=0;//下点不可能比上点还靠上}if(Right_Down_Find<=Right_Up_Find){Right_Down_Find=0;//下点不可能比上点还靠上}*/
            
            Left_Down_Find = (Left_Down_Find <= Left_Up_Find) ? 0 : Left_Down_Find;
            Right_Down_Find = (Right_Down_Find <= Right_Up_Find) ? 0 : Right_Down_Find;//上段的简化
            
            if(Left_Down_Find!=0 && Right_Down_Find!=0)
            {//四个点都在，无脑连线，这种情况显然很少
                Left_Add_Line (Left_Line [Left_Up_Find ],Left_Up_Find ,Left_Line [Left_Down_Find ] ,Left_Down_Find);
                Right_Add_Line(Right_Line[Right_Up_Find],Right_Up_Find,Right_Line[Right_Down_Find],Right_Down_Find);
            }
            else if(Left_Down_Find==0&&Right_Down_Find!=0)//11//这里使用的都是斜率补线
            {//三个点                                     //01
                Lengthen_Left_Boundry(Left_Up_Find-1,MT9V03X_H-1);
                Right_Add_Line(Right_Line[Right_Up_Find],Right_Up_Find,Right_Line[Right_Down_Find],Right_Down_Find);
            }
            else if(Left_Down_Find!=0&&Right_Down_Find==0)//11
            {//三个点                                     //10
                Left_Add_Line (Left_Line [Left_Up_Find ],Left_Up_Find ,Left_Line [Left_Down_Find ] ,Left_Down_Find);
                Lengthen_Right_Boundry(Right_Up_Find-1,MT9V03X_H-1);
            }
            else if(Left_Down_Find==0&&Right_Down_Find==0)//11
            {//就俩上点                                   //00
                Lengthen_Left_Boundry (Left_Up_Find-1,MT9V03X_H-1);
                Lengthen_Right_Boundry(Right_Up_Find-1,MT9V03X_H-1);
            }
        }
        else
        {
            Cross_Flag=0;
        }
    }

}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     摄像头误差获取
  @param     null
  @return    获取到的误差
  Sample     err=Err_Sum();
  @note      加权取平均
-------------------------------------------------------------------------------------------------------------------*/
float Err_Sum(void)
{
    int i;
    float err=0;
    float weight_count=0;
    //常规误差
    for(i=MT9V03X_H-1;i>=MT9V03X_H-Search_Stop_Line-1;i--)//常规误差计算
    {
        err+=(MT9V03X_W/2-((Left_Line[i]+Right_Line[i])>>1))*Weight[i];//右移1位，等效除2
        weight_count+=Weight[i];
    }
    err=err/weight_count;
//    if(Island_State!=0)//环岛取固定行数
//    {
//           for(i=51;i<=55;i++)
//           {
//               err+=(MT9V03X_W/2-((Left_Line[i]+Right_Line[i])>>1));
//           }
//        err=(float)err/5.0;
//    }
    return err;
}