#include "Trace.h"

/*tft180_x_max = 160;
tft180_y_max = 128;
MT9V03X_W = (160)       
MT9V03X_H = (128)*/
uint8 search_line_end = 10;     //������ֹ��
uint8 image_yuanshi[MT9V03X_H][MT9V03X_W];//��������ͷ��ͼ������
uint8 image_01[MT9V03X_H][MT9V03X_W];

extern const uint8 Image_Flags[][9][8];     //����ͼ�ϵ����ֱ��
uint8 image_two_value[MT9V03X_H][MT9V03X_W];//��ֵ�����ԭ����
volatile int Left_Line[MT9V03X_H]; //���������
volatile int Right_Line[MT9V03X_H];//�ұ�������
volatile int Mid_Line[MT9V03X_H];  //��������
volatile int Road_Wide[MT9V03X_H]; //��������
volatile int White_Column[MT9V03X_W];    //ÿ�а��г���
volatile int Search_Stop_Line;     //������ֹ��,ֻ��¼���ȣ���Ҫ������Ҫ����Ұ�߶ȼ�ȥ��ֵ
volatile int Boundry_Start_Left;   //���ұ߽���ʼ��
volatile int Boundry_Start_Right;  //��һ���Ƕ��ߵ�,����߽���ʼ��
volatile int Left_Lost_Time;       //�߽綪����
volatile int Right_Lost_Time;
volatile int Both_Lost_Time;//����ͬʱ������
//ʮ��
volatile int Cross_Flag=0;
volatile int Left_Down_Find=0; //ʮ��ʹ�ã��ҵ�����������û�ҵ�����0
volatile int Left_Up_Find=0;   //�ĸ��յ��־
volatile int Right_Down_Find=0;
volatile int Right_Up_Find=0;
volatile int Island_State=0;     //����״̬��־
volatile int Ramp_Flag=0;//�µ���־
volatile float Err=0;     //����ͷ���
volatile int Zebra_Stripes_Flag=0;

int Longest_White_Column_Left[2]; //�����,[0]������еĳ��ȣ�Ҳ����Search_Stop_Line������ֹ�У�[1���ǵ�ĳ��
int Longest_White_Column_Right[2];//�����,[0]������еĳ��ȣ�Ҳ����Search_Stop_Line������ֹ�У�[1���ǵ�ĳ��
int Left_Lost_Flag[MT9V03X_H] ; //�������飬������1��û������0
int Right_Lost_Flag[MT9V03X_H]; //�Ҷ������飬������1��û������0

void Zebra_Stripes_Detect(void);
void Cross_Detect(void);

const uint8 Standard_Road_Wide[MT9V03X_H]=//��׼����
{ 10, 12, 14, 16, 18, 20, 22, 24, 26, 28,
  30, 32, 34, 36, 38, 40, 42, 44, 46, 48,
  50, 52, 54, 56, 58, 60, 62, 64, 66, 68,
  70, 72, 74, 76, 78, 80, 82, 84, 86, 88,
  90, 92, 94, 96, 98,100,102,104,106,108,
 110,112,114,116,118,120,122,124,126,128,
 130,132,134,136,138,140,142,144,146,148};

struct ROAD_TYPE road_type = 
{
        .straight      = 0,//ֱ��
        .bend          = 0,//���
        .Ramp          = 0,//�µ�
        .Cross         = 0,//ʮ��
        .L_Cross       = 0,
        .R_Cross       = 0,
        .LeftCirque    = 0,//��Բ��
        .RightCirque   = 0,
        .Fork          = 0,//��·

};


struct STATUS_TYPE status_type = 
{
        .start_pick    = 0,//��ʼ��Ƭ
        .stop_pick     = 0,
        .start_lay_W   = 0,//��ʼ������ĸС�࿨Ƭ
        .stop_lay_W    = 0,
        .start_lay_F   = 0,//��ʼ�������ִ��࿨Ƭ
        .stop_lay_F    = 0,
};

const uint8 Weight[MT9V03X_H]= //�ѽ�MT9V03X_H����Ϊ90
{
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //ͼ����Զ��00 ����09 ��Ȩ��
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //ͼ����Զ��10 ����19 ��Ȩ��
        1, 1, 1, 1, 1, 1, 1, 3, 4, 5,              //ͼ����Զ��20 ����29 ��Ȩ��
        6, 7, 9,11,13,15,17,19,20,20,              //ͼ����Զ��30 ����39 ��Ȩ��
       19,17,15,13,11, 9, 7, 5, 3, 1,              //ͼ����Զ��40 ����49 ��Ȩ��
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //ͼ����Զ��50 ����59 ��Ȩ��
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1,              //ͼ����Զ��60 ����69 ��Ȩ��

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
  @brief     �Ƚϴ�С
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
//  �洢��һ�������飬��������HighΪ120��WidthΪ188��ˢ����Ϊ50������֪��ˢ����Ϊʲô50
//  @param      image  ͼ������
//  @param      clo    ��
//  @param      row    ��
//  @param      pixel_threshold ��ֵ����
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
//  ���ֵ����ֵ
//  @param      image  ͼ������
//  @param      clo    ��
//  @param      row    ��
//  @param      pixel_threshold ��ֵ����
//-------------------------------------------------------------------------------------------------------------------
uint8 Threshold_Deal(uint8* image, uint16 col, uint16 row, uint32 pixel_threshold)
{
#define GrayScale 256//�Ҷȼ���GrayScaleΪ256
    uint16 width = col;
    uint16 height = row;
    int pixelCount[GrayScale];//���ؼ����������±��ʾ�Ҷȼ�������ֵ�������
    float pixelPro[GrayScale];
    int i, j;
    int pixelSum = width * height;//���غ�
    uint8 threshold = 0;
    uint8* data = image;  //ָ���������ݵ�ָ��
    for (i = 0; i < GrayScale; i++)
    {
        pixelCount[i] = 0;
        pixelPro[i] = 0;
    }

    uint32 gray_sum = 0;
    //ͳ�ƻҶȼ���ÿ������������ͼ���еĸ���
    for (i = 0; i < height; i += 1)
    {
        for (j = 0; j < width; j += 1)
        {
            // if((sun_mode&&data[i*width+j]<pixel_threshold)||(!sun_mode))
            //{����ͼ��Ŀ��Ϊwidth���߶�Ϊheight��
            //��ô��i�е�j�е�������һά�����е���������ͨ�����¹�ʽ����õ���
            //index=i��width+j
          //data[i]�����ֵΪ�Ҷȼ�
            pixelCount[(int)data[i * width + j]]++;  //����ǰ��ĻҶȼ�������ֵ��Ϊ����������±�
            gray_sum += (int)data[i * width + j];  //�Ҷ�ֵ�ܺ�
            //}
        }
    }

    //����ÿ������ֵ�ĵ�������ͼ���еı���
    for (i = 0; i < GrayScale; i++)
    {
        pixelPro[i] = (float)pixelCount[i] / pixelSum;
    }

    //�����Ҷȼ�[0,255]
    float w0, w1, u0tmp, u1tmp, u0, u1, u, deltaTmp, deltaMax = 0;
    w0 = w1 = u0tmp = u1tmp = u0 = u1 = u = deltaTmp = 0;
    for (j = 0; j < pixel_threshold; j++)// pixel_threshold��ֵ���룬�����Լ�����
    {
       //pixelPro[j]����ֵ�ĵ�������ͼ���еı���
        //�������ֵ�Ȩ�غ���ʱֵ
        w0 += pixelPro[j];  //��������ÿ���Ҷ�ֵ�����ص���ռ����֮�� ���������ֵı�����Ӧ����Ȩ��
        u0tmp += j * pixelPro[j];  //�������� ÿ���Ҷ�ֵ�ĵ�ı��� *�Ҷ�ֵ
        
        //ǰ�����ֵ�Ȩ�غ���ʱֵ
        w1 = 1 - w0;
        u1tmp = gray_sum / pixelSum - u0tmp;

        //ƽ��ֵ����Ȩ��
        u0 = u0tmp / w0;    //����ƽ���Ҷ�
        u1 = u1tmp / w1;    //ǰ��ƽ���Ҷ�
        u = u0tmp + u1tmp;  //ȫ��ƽ���Ҷ�
        
        /*�ⲿ�ּ����˱������ֵ�ƽ���Ҷ���ȫ��ƽ���Ҷ�֮��Ĳ����ƽ��������2����
        �����Ա������ֵ�Ȩ�ء����ֵԽ���ʾ����������ȫ��ƽ���Ҷ�֮��Ĳ���Խ��
        Ҳ����˵����ǰ��ֵ���ڱ������ֵķָ�Ч��Խ�á�*/
        deltaTmp = w0 * pow((u0 - u), 2) + w1 * pow((u1 - u), 2);//���ֵԽ���ʾ��ǰ��ֵ��������ͼ��ķָ�Ч��Խ��
        if (deltaTmp > deltaMax)//���¶�ֵ��ֵ
        {
            deltaMax = deltaTmp;
            threshold = (uint8)j;     //��������û��ǿ������ת����,���Լ��ӵ�
        }
        if (deltaTmp < deltaMax)
        {
            break;
        }
    }
    return threshold;
}


//-------------------------------------------------------------------------------------------------------------------
//  ��ֵ��
//  ����ֵ��ͼ������ת����image_01[y][x]
// �˲�
//-------------------------------------------------------------------------------------------------------------------
uint8 Threshold;  //��ֵ
uint8 Threshold_static = 150;   //��ֵ��̬����160 220  230            ���죺200   ���ϣ�230  A:200
uint16 Threshold_detach = 100;  //�����㷨�ָ���ֵ(��ǿԽǿ,��ֵԽ��)   ���죺300
void Get01change_Dajin(void)//�������ͼ����ж�ֵ��
{               //���ڼ���ͼ�����ֵ
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

            if (image_yuanshi[y][x] >thre)         //��ֵԽ����ʾ������Խ�࣬��ǳ��ͼ��Ҳ����ʾ����
                image_01[y][x] = 255;  //��
            else
                image_01[y][x] = 0;  //��  ��ֵ�����
        }
    }
     Pixle_Filter();//��ֵ��֮������˲�
}


//-------------------------------------------------------------------------------------------------------------------
//�����˲�,ȥ��ż�����ֵ����,Ч�����ޣ�
//-------------------------------------------------------------------------------------------------------------------
void Pixle_Filter(void)
{

    for (uint8 height = 10; height < MT9V03X_H-10; height++)
    {
        for (uint8 width = 10; width < MT9V03X_W -10; width = width + 1)
        {
            if ((image_01[height][width] == 0) && (image_01[height - 1][width] + image_01[height + 1][width] +image_01[height][width + 1] + image_01[height][width - 1] >=3*255))
            { //һ���ڵ���������ҵİ׵���ڵ����������������Ϊ��
                image_01[height][width] = 255;//��1��Ϊ��255
            }//һ���׵���������ҵĺڵ���ڵ����������������Ϊ��                                        
            else if((image_01[height][width] == 255)&&(image_01[height-1][width]+image_01[height+1][width]+image_01[height][width+1]+image_01[height][width-1]<=255))
            {
                image_01[height][width] =0;
            }
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     ˫�����Ѳ��
  @param     null
  @return    null
  Sample     Longest_White_Column_Left();
  @note      �����Ѳ�ߣ�Ѱ�ҳ�ʼ�߽磬���ߣ�����еȻ���Ԫ�أ�������ȡ��Щ��������������ʶ��
-------------------------------------------------------------------------------------------------------------------*/
void Longest_White_Column()//�����Ѳ��
{
    int i, j;
    int start_column=20;//����е�������ʼ�У��ڶ�ʮ�п�ʼ��
    int end_column=MT9V03X_W-20;//����е����������У���W-20������
    int left_border = 0, right_border = 0;//��ʱ�洢����λ��
    Longest_White_Column_Left[0] = 0;//�����,[0]������еĳ��ȣ�[1���ǵ�ĳ��
    Longest_White_Column_Left[1] = 0;//�����,[0]������еĳ��ȣ�[1���ǵ�ĳ��
    Longest_White_Column_Right[0] = 0;//�����,[0]������еĳ��ȣ�[1���ǵ�ĳ��
    Longest_White_Column_Right[1] = 0;//�����,[0]������еĳ��ȣ�[1���ǵ�ĳ��
    Right_Lost_Time = 0;    //�߽綪����
    Left_Lost_Time  = 0;
    Boundry_Start_Left  = 0;//��һ���Ƕ��ߵ�,����߽���ʼ��
    Boundry_Start_Right = 0;
    Both_Lost_Time = 0;//����ͬʱ������
    //tft180_draw_line ( MT9V03X_W/2, 0, MT9V03X_W/2, MT9V03X_H-1, RGB565_RED);

    for (i = 0; i <=MT9V03X_H-1; i++)//��������
    {
        Right_Lost_Flag[i] = 0;
        Left_Lost_Flag[i] = 0;
        Left_Line[i] = 0;//���������
        Right_Line[i] = MT9V03X_W-1;
    }
    for(i=0;i<=MT9V03X_W-1;i++)//ÿ�а��г�����0
    {
        White_Column[i] = 0;
    }

   //������Ҫ������з�Χ�����޶�
    /*//����3״̬��Ҫ�ı������Ѱ�ҷ�Χ
    if(Right_Island_Flag==1)//�һ�
    {
        if(Island_State==3)
        {
            start_column=40;//����е���������
            end_column=MT9V03X_W-20;
        }
    }
    else if(Left_Island_Flag==1)//��
    {
        if(Island_State==3)
        {
            start_column=20;
            end_column=MT9V03X_W-40;
        }
    }*/
    //�����ң��������ϣ�����ȫͼ��¼��Χ�ڵ�ÿһ�а׵�����
    
    for (j =start_column; j<=end_column; j++)
    {
        for (i = MT9V03X_H-1; i >= 10; i--)
        {
            if(image_01[i][j] == IMG_BLACK)
                break;//�����ڲ�for
            else
                White_Column[j]++;//����λ��������һ�У������ֵ�����Ӧ�еİ׵�����
        }
    }

    //����������������
    Longest_White_Column_Left[0] =0;/*�����,����λ��ʾ����еĳ��ȣ���һλ����е�����
    Ҳ����Search_Stop_Line������ֹ�У�[1���ǵ�ĳ��*/
    for(i=start_column;i<=end_column;i++)
    {
        if (Longest_White_Column_Left[0] < White_Column[i])//�������һ��
        {
            Longest_White_Column_Left[0] = White_Column[i];//��0���ǰ��г��ȣ�����ǰ�еİ��г��ȸ�ֵ��
                                                             //Longest_White_Column_Left�ĵ�0λ
            Longest_White_Column_Left[1] = i;              //��1�����±꣬��j��
        }
    }
    //���ҵ������ұ������
    Longest_White_Column_Right[0] = 0;//��0���ǰ��г���
    for(i=end_column;i>=start_column;i--)//��������ע���������ҵ���������λ�þͿ���ͣ��
    {
        if (Longest_White_Column_Right[0] < White_Column[i])//�������һ��
        {
            Longest_White_Column_Right[0] = White_Column[i];//��0���ǰ��г���
            Longest_White_Column_Right[1] = i;              //��1�����±꣬��j��
        }
    }

//���������ж�Ϊ������ǰ����е����г��ȣ���ΪӦ�ÿ��Խ������̣��Ͼ�ÿ�����������ͬ
//����ֱ������ͼ����ߣ��������������Ԫ�������
    Search_Stop_Line = Longest_White_Column_Left[0];//������ֹ��ѡȡ����������𲻴�����������������һ����
    //����������
    //tft180_draw_line ( 0, Search_Stop_Line, MT9V03X_W-1, Search_Stop_Line, RGB565_RED);
    //����Ļ�������ߴ���ʾ�����ڵ�������ͷ
    
    
    for (i = MT9V03X_H - 1; i >=MT9V03X_H-Search_Stop_Line; i--)//����Ѳ��
    {//               ��ʾ����е���������ţ������ɨ��
        for (j = Longest_White_Column_Right[1]; j <= MT9V03X_W - 1 ; j++)
        {     
            if (image_01[i][j] ==IMG_WHITE && image_01[i][j + 1] == IMG_BLACK && image_01[i][j + 2] == IMG_BLACK)//�׺ںڣ��ҵ��ұ߽�
            {
                right_border = j;//��������λ�ã�ֵ��ʾ�ڼ��У�
                Right_Lost_Flag[i] = 0; //�Ҷ������飬��ʾ��i�ж����� ������1����������0
                break;
            }
            else if(j>=MT9V03X_W-1)//û�ҵ��ұ߽磬����Ļ���Ҹ�ֵ���ұ߽�
            {
                right_border = j;
                Right_Lost_Flag[i] = 1; //�Ҷ������飬������1����������0
                break;
            }
        }
        for (j = Longest_White_Column_Left[1]; j >= 0; j--)//�����ɨ��
        {
            if (image_01[i][j] ==IMG_WHITE && image_01[i][j - 1] == IMG_BLACK && image_01[i][j - 2] == IMG_BLACK)//�ںڰ���Ϊ������߽�
            {
                left_border = j;
                Left_Lost_Flag[i] = 0; //�������飬������1����������0
                break;
            }
            else if(j<=0)
            {
                left_border = j;//�ҵ�ͷ��û�ҵ��ߣ��Ͱ���Ļ�����ҵ����߽�
                Left_Lost_Flag[i] = 1; //�������飬������1����������0
                break;
            }
        }
        
        //i��ʾ�У�ֵ������
        Left_Line [i] = left_border;       //����������飬����ֵ��ʾ��ǰ�߽�ĵڼ���
        Right_Line[i] = right_border;      //�ұ���������//���Կ����ڴ˻��߽�
        Mid_Line[i] = ( Left_Line[i] +Right_Line[i])/2; 
        
        //tft180_draw_point(left_border,i,RGB565_BLUE);//�����
        //tft180_draw_point(right_border,i,RGB565_BLUE);//�ұ���
        //tft180_draw_point(Mid_Line[i],i,RGB565_PURPLE);
        //offset +=offset_quanzhong[MT9V03X_H-30 -i] *(Mid_Line[i] -MT9V03X_W/2);
    }
      
    for (i = MT9V03X_H - 1; i >= 0; i--)//�������ݳ�������
    {
        if (Left_Lost_Flag[i]  == 1)//���߶�����
            Left_Lost_Time++;
        if (Right_Lost_Flag[i] == 1)
            Right_Lost_Time++;
        if (Left_Lost_Flag[i] == 1 && Right_Lost_Flag[i] == 1)//˫�߶�����
            Both_Lost_Time++;
        //Boundry_Start_Left ���ұ߽���ʼ�㣬ֻ��¼һ��
        if (Boundry_Start_Left ==  0 && Left_Lost_Flag[i]  != 1)//��¼��һ���Ƕ��ߵ㣬�߽���ʼ��
            Boundry_Start_Left = i;
        if (Boundry_Start_Right == 0 && Right_Lost_Flag[i] != 1)
            Boundry_Start_Right = i;
        Road_Wide[i]=Right_Line[i]-Left_Line[i];//·�Ŀ��
        
//        tft180_show_string(0,100,"Road_Wide=");
//        tft180_show_int(80,100,(int32)Road_Wide[i],4);
//        tft180_show_string(0,130,"BothLostTime=");
//        tft180_show_int(95,130,(int32) Both_Lost_Time,3);
        
    }
    
    
    
    
    //����3״̬�ı�߽磬���������������Ϊ�����ϵ���������ǲ���Ҫ��Щ�����
    /*f(Island_State==3||Island_State==4)
    {
        if(Right_Island_Flag==1)//�һ�
        {
            for (i = MT9V03X_H - 1; i >= 0; i--)//�ұ�ֱ��д�ڱ���
            {
                Right_Line[i]=MT9V03X_W-1;
            }
        }
        else if(Left_Island_Flag==1)//��
        {
            for (i = MT9V03X_H - 1; i >= 0; i--)//���ֱ��д�ڱ���
            {
                Left_Line[i]=0;      //�ұ���������
            }
        }
    }*/
}



/*-------------------------------------------------------------------------------------------------------------------
  @brief     ����
  @param     ���ߵ���㣬�յ�
  @return    null
  Sample     Left_Add_Line(int x1,int y1,int x2,int y2);
  @note      ����ֱ���Ǳ߽磬������ǿ��Ŷȸߵ�,��Ҫ�Ҳ�
����ʱ�ðɣ���ʱ����
-------------------------------------------------------------------------------------------------------------------*/
void Left_Add_Line(int x1,int y1,int x2,int y2) //����,�����Ǳ߽�
{
    int i,a1,a2;
    int hx;
//���������ͼ���������Ϊͼ���������С����Ϊ0
    x1 = my_max(0, my_min(x1, MT9V03X_W-1));
    y1 = my_max(0, my_min(y1, MT9V03X_H-1));
    x2 = my_max(0, my_min(x2, MT9V03X_W-1));
    y2 = my_max(0, my_min(y2, MT9V03X_H-1));

    a1=y1;
    a2=y2;
    
    //if(a1>a2)//���껥��{ max=a1;a1=a2;a2=max;}
    if(a1 > a2) 
    { a1 ^= a2; a2 ^= a1; a1 ^= a2; }
    
    for(i=a1;i<=a2;i++)//����б�ʲ��߼���
    {
        hx=(i-y1)*(x2-x1)/(y2-y1)+x1;
        //if(hx>=MT9V03X_W)hx=MT9V03X_W;else if(hx<=0)hx=0;
        hx = my_max(0, my_min(hx, MT9V03X_W));
        Left_Line[i]=hx; //i��ʾ�У�ֵ������
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     �Ҳ���
  @param     ���ߵ���㣬�յ�
  @return    null
  Sample     Right_Add_Line(int x1,int y1,int x2,int y2);
  @note      ����ֱ���Ǳ߽磬������ǿ��Ŷȸߵģ���Ҫ�Ҳ�
-------------------------------------------------------------------------------------------------------------------*/
void Right_Add_Line(int x1,int y1,int x2,int y2)//�Ҳ���,�����Ǳ߽�
{
    int i,a1,a2;
    int hx;

    x1 = my_max(0, my_min(x1, MT9V03X_W-1));
    y1 = my_max(0, my_min(y1, MT9V03X_H-1));
    x2 = my_max(0, my_min(x2, MT9V03X_W-1));
    y2 = my_max(0, my_min(y2, MT9V03X_H-1));
    
    a1=y1;
    a2=y2;
    //if(a1>a2)//���껥��{max=a1;a1=a2;a2=max;}
    if(a1 > a2) 
    {
    a1 ^= a2;
    a2 ^= a1;
    a1 ^= a2;
    }
    for(i=a1;i<=a2;i++)//����б�ʲ��߼���
    {
        hx=(i-y1)*(x2-x1)/(y2-y1)+x1;
        //if(hx>=MT9V03X_W)hx=MT9V03X_W;else if(hx<=0)hx=0; 
        hx = my_max(0, my_min(hx, MT9V03X_W)); 
        Right_Line[i]=hx;
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     ������������յ��������ʹ��֮��
             Left_down_Find
             Right_down_Find������ֵ�����������ҹյ��ڵ�����
             ��ʮ��ʹ��
  @param     �����ķ�Χ��㣬�յ�
  @return    �޸�����ȫ�ֱ���
             Right_Down_Find=0;
             Left_Down_Find=0;
  Sample     Find_Down_Point(int start,int end)
  @note      ������֮��鿴��Ӧ�ı�����ע�⣬û�ҵ�ʱ��Ӧ��������0
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
    /*if(start>=MT9V03X_H-1-5)//����5�����ݲ��ȶ���������Ϊ�߽�����жϣ�����
        start=MT9V03X_H-1-5;*///�����᲻�����ø���ʵ��
      
    /*if(end<=MT9V03X_H-Search_Stop_Line)
        end=MT9V03X_H-Search_Stop_Line;
    if(end<=5)
       end=5;*/
    start = my_min(MT9V03X_H - 1 - 5, start);
    if(end <= MT9V03X_H - Search_Stop_Line || end <= 5)
        {end = my_max(MT9V03X_H - Search_Stop_Line, 5);}
    
    for(int i=start;i>=end;i--)
    {
        if(Left_Down_Find==0&&//ֻ�ҵ�һ�����������ĵ�
                 //i��ʾ�У�ֵ������
           abs(Left_Line[i]-Left_Line[i+1])<=5&&//�ǵ����ֵ���Ը���
           abs(Left_Line[i+1]-Left_Line[i+2])<=5&&
           abs(Left_Line[i+2]-Left_Line[i+3])<=5&&
              (Left_Line[i]-Left_Line[i-2])>=8&&
              (Left_Line[i]-Left_Line[i-3])>=15&&
              (Left_Line[i]-Left_Line[i-4])>=15)
        {
            Left_Down_Find=i;//��ȡ��������
        }
        if(Right_Down_Find==0&&//ֻ�ҵ�һ�����������ĵ�
           abs(Right_Line[i]-Right_Line[i+1])<=5&&//�ǵ����ֵ���Ը���
           abs(Right_Line[i+1]-Right_Line[i+2])<=5&&
           abs(Right_Line[i+2]-Right_Line[i+3])<=5&&
              (Right_Line[i]-Right_Line[i-2])<=-8&&
              (Right_Line[i]-Right_Line[i-3])<=-15&&
              (Right_Line[i]-Right_Line[i-4])<=-15)
        {
            Right_Down_Find=i;
        }
        if(Left_Down_Find!=0 && Right_Down_Find!=0)//�����ҵ����˳�
        {
            break;
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     ������������յ��������ʹ��֮��
             Left_Up_Find
             Right_Up_Find������ֵ�����������ҹյ��ڵ�����
            ��ʮ��ʹ��
  @param     �����ķ�Χ��㣬�յ�
  @return    �޸�����ȫ�ֱ���
             Left_Up_Find=0;
             Right_Up_Find=0;
  Sample     Find_Up_Point(int start,int end)
  @note      ������֮��鿴��Ӧ�ı�����ע�⣬û�ҵ�ʱ��Ӧ��������0
-------------------------------------------------------------------------------------------------------------------*/
void Find_Up_Point(int start,int end)//�����������
{
    Left_Up_Find=0;//��־
    Right_Up_Find=0;
/* if(start<end){t=start;start=end;end=t;} */
   
    if(start<end) //����ֵ
    { start ^= end; end ^= start; start ^= end;}
    
   /* if(end<=MT9V03X_H-Search_Stop_Line)
        end=MT9V03X_H-Search_Stop_Line;
    if(end<=5)                        //��ʱ����зǳ�����ҲҪ�������ֵ㣬��ֹ����Խ��
        end=5;
    if(start>=MT9V03X_H-1-5)              //����5�����ݲ��ȶ���������Ϊ�߽�����жϣ�����
        start=MT9V03X_H-1-5;*/
    if(end <= MT9V03X_H - Search_Stop_Line || end <= 5)
        {end = my_max(MT9V03X_H - Search_Stop_Line, 5);}
    start = my_min(MT9V03X_H - 1 - 5, start);
    for(int i=start;i>=end;i--)
    {
        if(Left_Up_Find==0&&//ֻ�ҵ�һ�����������ĵ�
            //i��ʾ�У�ֵ������
           abs(Left_Line[i]-Left_Line[i-1])<=5&&//��ߵ�i��-��ߵ�i�е�������֮���в�ֵС��5
           abs(Left_Line[i-1]-Left_Line[i-2])<=5&&
           abs(Left_Line[i-2]-Left_Line[i-3])<=5&&
              (Left_Line[i]-Left_Line[i+2])>=8&&
              (Left_Line[i]-Left_Line[i+3])>=15&&
              (Left_Line[i]-Left_Line[i+4])>=15)
        {
            Left_Up_Find=i;//��ȡ��������
        }
        if(Right_Up_Find==0&&//ֻ�ҵ�һ�����������ĵ�
           abs(Right_Line[i]-Right_Line[i-1])<=5&&//��������λ�ò��
           abs(Right_Line[i-1]-Right_Line[i-2])<=5&&
           abs(Right_Line[i-2]-Right_Line[i-3])<=5&&
              (Right_Line[i]-Right_Line[i+2])<=-8&&
              (Right_Line[i]-Right_Line[i+3])<=-15&&
              (Right_Line[i]-Right_Line[i+4])<=-15)
        {
            Right_Up_Find=i;//��ȡ��������
        }
        if(Left_Up_Find!=0&&Right_Up_Find!=0)//���������ҵ��ͳ�ȥ
        {
            break;
        }
    }
    if(abs(Right_Up_Find-Left_Up_Find)>=30)//����˺�ѹ�����Ϊ����
    {
        Right_Up_Find=0;
        Left_Up_Find=0;
    }
}
/*-------------------------------------------------------------------------------------------------------------------
���й��ܺ���д���ٽ���Ԫ���ж�
---------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------------------------------------------------------------------------------
  @brief     ��߽��ӳ�
  @param     �ӳ���ʼ�������ӳ���ĳ��
  @return    null
  Sample     Stop_Detect(void)
  @note      ����ʼ��������5���㣬���б�ʣ������ӳ���ֱ��������
-------------------------------------------------------------------------------------------------------------------*/
void Lengthen_Left_Boundry(int start,int end)
{
    int i,t;
    float k=0;
    //��ʼ��λ��У�����ų�����Խ��Ŀ���
    start = my_max(0, my_min(MT9V03X_H - 1, start));
    end = my_max(0, my_min(MT9V03X_H - 1, end));
    
    if(end<start)//���껥��
    {
        t=end;
        end=start;
        start=t;
    }

    if(start<=5)//��Ϊ��Ҫ�ڿ�ʼ��������3���㣬������ʼ����ڿ��ϣ��������ӳ���ֻ��ֱ������
    {
         Left_Add_Line(Left_Line[start],start,Left_Line[end],end);
    }

    else
    {
        k=(float)(Left_Line[start]-Left_Line[start-4])/5.0;//�����k��1/б��
        for(i=start;i<=end;i++)
        {
            Left_Line[i]=(int)((i-start)*k+Left_Line[start]);//(x=(y-y1)*k+x1),��бʽ����
            
            /*if(Left_Line[i]>=MT9V03X_W-1){Left_Line[i]=MT9V03X_W-1;}
            else if(Left_Line[i]<=0){Left_Line[i]=0;}*/
            Left_Line[i] = my_max(0, my_min(MT9V03X_W - 1, Left_Line[i]));
            
        }
    }
}

/*-------------------------------------------------------------------------------------------------------------------
  @brief     ����߽��ӳ�
  @param     �ӳ���ʼ�������ӳ���ĳ��
  @return    null
  Sample     Stop_Detect(void)
  @note      ����ʼ��������3���㣬���б�ʣ������ӳ���ֱ��������
-------------------------------------------------------------------------------------------------------------------*/
void Lengthen_Right_Boundry(int start,int end)
{
    int i,t;
    float k=0;
   /* if(start>=MT9V03X_H-1)//��ʼ��λ��У�����ų�����Խ��Ŀ���
start=MT9V03X_H-1;else if(start<=0)start=0;if(end>=MT9V03X_H-1)end=MT9V03X_H-1;else if(end<=0)end=0;*/
    start = my_max(0, my_min(MT9V03X_H - 1, start));
    end = my_max(0, my_min(MT9V03X_H - 1, end));
    if(end<start)//++���ʣ����껥��
    {
        t=end;
        end=start;
        start=t;
    }

    if(start<=5)//��Ϊ��Ҫ�ڿ�ʼ��������3���㣬������ʼ����ڿ��ϣ��������ӳ���ֻ��ֱ������
    {
        Right_Add_Line(Right_Line[start],start,Right_Line[end],end);
    }
    else
    {
        k=(float)(Right_Line[start]-Right_Line[start-4])/5.0;//�����k��1/б��
        for(i=start;i<=end;i++)
        {
            Right_Line[i]=(int)((i-start)*k+Right_Line[start]);//(x=(y-y1)*k+x1),��бʽ����
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
  @brief     �����߼��
  @param     null
  @return    null
    ��Ҫ��������ͷ��λ���жϾ������ֵ
  Sample     Zebra_Stripes_Detect(void)
  @note      �߽���ʼ���£�����нϳ���������ȹ�խ���Ҹ�����������
-------------------------------------------------------------------------------------------------------------------*/

void Zebra_Stripes_Detect(void)
{
    int i=0,j=0;
    int change_count=0;//�������
    int start_line=0;
    int endl_ine=0;
    int narrow_road_count=0;
    if(Cross_Flag!=0||(1<=Ramp_Flag&&Ramp_Flag<=3)
/*||Zebra_Stripes_Flag!=0||Stop_Flag!=0 ||Electromagnet_Flag!=0||Barricade_Flag*/!=0)//Ԫ�ػ��⣬����ʮ�֣����ǣ������µ�������ͣ��
    {
        return;
    }

    ////����仯�а�����
    if(Search_Stop_Line>=60&&//���������д���60��������д��ڵ���30��С�ڵ���W-30
       30<=Longest_White_Column_Left[1]&&Longest_White_Column_Left[1]<=MT9V03X_W-30&&
       30<=Longest_White_Column_Right[1]&&Longest_White_Column_Right[1]<=MT9V03X_W-30&&
         //���ұ߽���ʼ�㣬������
       Boundry_Start_Left>=MT9V03X_H-15&&Boundry_Start_Right>=MT9V03X_H-15)
    {    //��ֹ�г�������е�λ�������ĸ������߽���ʼ�㿿��
        for(i=65;i>=20;i--)//�ڿ��µ��������Ѱ��������ȹ�խ�ĵط�
        {//      ��׼����                �����߽�������Ŀ��
            if( (Standard_Road_Wide[i]-Road_Wide[i]) > 10 )//��������
            {
                narrow_road_count++;//���������խ������Ϊ�ǰ�����
                if(narrow_road_count>=5)
                {
                    start_line=i;//��¼������Ⱥ�խ��λ�� ����
                    break;//������ǰforѭ��
                }
            }
        }
    }
    if(start_line!=0)//���������խ����������խ��λ��Ϊ���ģ�����һ����Χ�������������
    {
        start_line=start_line+8;
        endl_ine=start_line-15;
        if(start_line>=MT9V03X_H-1)//�޷���������ֹ����Խ��
        {
            start_line=MT9V03X_H-1;
        }
        //����mymax mymin
        if(endl_ine<=0)//�޷���������ֹ����Խ��
        {
            endl_ine=0;
        }
        for(i=start_line;i>=endl_ine;i--)//�������������
        {
            for(j=Left_Line[i];j<=Right_Line[i];j++)
            {
                if(image_two_value[i][j+1]-image_two_value[i][j]!=0)
                {
                    change_count++;

                }
            }
        }
//        ips200_show_uint(0*16,100,change_count,5);//debugʹ�ã��鿴��������������Ӧ����
    }
 //�������򣬱�����bug��debugʹ��
//        Draw_Line( Left_Line[start_line], start_line, Left_Line[endl_ine], endl_ine);
//        Draw_Line( Left_Line[start_line], start_line, Right_Line[start_line], start_line);
//        Draw_Line(Right_Line[endl_ine], endl_ine, Right_Line[start_line], start_line);
//        Draw_Line(Right_Line[endl_ine], endl_ine, Left_Line[endl_ine], endl_ine);
//        ips200_draw_line ( Left_Line[start_line], start_line, Left_Line[endl_ine], endl_ine, RGB565_RED);
//        ips200_draw_line ( Left_Line[start_line], start_line, Right_Line[start_line], start_line, RGB565_RED);
//        ips200_draw_line (Right_Line[endl_ine], endl_ine, Right_Line[start_line], start_line, RGB565_RED);
//        ips200_draw_line (Right_Line[endl_ine], endl_ine, Left_Line[endl_ine], endl_ine, RGB565_RED);

    if(change_count>30)//�������ĳһ��ֵ����Ϊ�ҵ��˰�����
    {
        Zebra_Stripes_Flag=1;
    }
}


/*-------------------------------------------------------------------------------------------------------------------
  @brief     ֱ�����
  @param     null
  @return    null
  Sample     Straight_Detect()��
  @note      ��������У��߽���ʼ�㣬������ʼ�㣬
-------------------------------------------------------------------------------------------------------------------*/
void Straight_Detect(void)
{
    //Straight_Flag=0;����ʱע��
    if(Search_Stop_Line>=65)//��ֹ�к�Զ
    {
        if(Boundry_Start_Left>=68&&Boundry_Start_Right>=65)//��ʼ�㿿��
        {
            /*if(-5<=Err&&Err<=5)//����С
            {
                Straight_Flag=1;//��Ϊ��ֱ��
            }*/
        }
    }
}






/*-------------------------------------------------------------------------------------------------------------------
  @brief     ʮ�ּ��
  @param     null
  @return    null
  Sample     Cross_Detect(void);
  @note      �����ĸ��յ��б����������ĸ��ǵ㣬�����ҵ��յ�ĸ��������Ƿ���
-------------------------------------------------------------------------------------------------------------------*/
void Cross_Detect()
{
    int down_search_start=0;//�µ�������ʼ��
    Cross_Flag=0;
    if(Island_State==0 && Ramp_Flag==0)//�뻷�����µ����⿪���µ���ɾ��
    {
        Left_Up_Find=0; //�ĸ��յ��־
        Right_Up_Find=0;
        if(Both_Lost_Time>=10)//������������ͼ���жϣ���ʮ�ֱض���˫�߶��ߣ�����˫�߶��ߵ�������ٿ�ʼ�ҽǵ�
        {
            Find_Up_Point( MT9V03X_H-1, 0 );//������������յ㣬��ʮ��ʹ�ã��ǲ��Ƿ�Χ̫���ˣ�
            //��һ����㣬�ζ����յ�
            if(Left_Up_Find==0 && Right_Up_Find==0)//ֻҪû��ͬʱ�ҵ������ϵ㣬ֱ�ӽ���
            {
                return;//
            }
        }
        if(Left_Up_Find!=0 && Right_Up_Find!=0)//�ҵ������ϵ㣬���ҵ�ʮ����
        {
            Cross_Flag=1;//��Ӧ��־λ�����ڸ�Ԫ�ػ����
            
            //��Left_Up_Find��Right_Up_Find�нϴ��ֵ����down_search_start
            //�������Ϲյ����꿿������Ϊ�µ����������
            //down_search_start �µ�������ʼ��
            down_search_start = Left_Up_Find>Right_Up_Find ? Left_Up_Find : Right_Up_Find;
            Find_Down_Point(MT9V03X_H-5,down_search_start+2);//���Ϲյ���2����Ϊ�µ�Ľ�ֹ��
/*if(Left_Down_Find<=Left_Up_Find){Left_Down_Find=0;//�µ㲻���ܱ��ϵ㻹����}if(Right_Down_Find<=Right_Up_Find){Right_Down_Find=0;//�µ㲻���ܱ��ϵ㻹����}*/
            
            Left_Down_Find = (Left_Down_Find <= Left_Up_Find) ? 0 : Left_Down_Find;
            Right_Down_Find = (Right_Down_Find <= Right_Up_Find) ? 0 : Right_Down_Find;//�϶εļ�
            
            if(Left_Down_Find!=0 && Right_Down_Find!=0)
            {//�ĸ��㶼�ڣ��������ߣ����������Ȼ����
                Left_Add_Line (Left_Line [Left_Up_Find ],Left_Up_Find ,Left_Line [Left_Down_Find ] ,Left_Down_Find);
                Right_Add_Line(Right_Line[Right_Up_Find],Right_Up_Find,Right_Line[Right_Down_Find],Right_Down_Find);
            }
            else if(Left_Down_Find==0&&Right_Down_Find!=0)//11//����ʹ�õĶ���б�ʲ���
            {//������                                     //01
                Lengthen_Left_Boundry(Left_Up_Find-1,MT9V03X_H-1);
                Right_Add_Line(Right_Line[Right_Up_Find],Right_Up_Find,Right_Line[Right_Down_Find],Right_Down_Find);
            }
            else if(Left_Down_Find!=0&&Right_Down_Find==0)//11
            {//������                                     //10
                Left_Add_Line (Left_Line [Left_Up_Find ],Left_Up_Find ,Left_Line [Left_Down_Find ] ,Left_Down_Find);
                Lengthen_Right_Boundry(Right_Up_Find-1,MT9V03X_H-1);
            }
            else if(Left_Down_Find==0&&Right_Down_Find==0)//11
            {//�����ϵ�                                   //00
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
  @brief     ����ͷ����ȡ
  @param     null
  @return    ��ȡ�������
  Sample     err=Err_Sum();
  @note      ��Ȩȡƽ��
-------------------------------------------------------------------------------------------------------------------*/
float Err_Sum(void)
{
    int i;
    float err=0;
    float weight_count=0;
    //�������
    for(i=MT9V03X_H-1;i>=MT9V03X_H-Search_Stop_Line-1;i--)//����������
    {
        err+=(MT9V03X_W/2-((Left_Line[i]+Right_Line[i])>>1))*Weight[i];//����1λ����Ч��2
        weight_count+=Weight[i];
    }
    err=err/weight_count;
//    if(Island_State!=0)//����ȡ�̶�����
//    {
//           for(i=51;i<=55;i++)
//           {
//               err+=(MT9V03X_W/2-((Left_Line[i]+Right_Line[i])>>1));
//           }
//        err=(float)err/5.0;
//    }
    return err;
}