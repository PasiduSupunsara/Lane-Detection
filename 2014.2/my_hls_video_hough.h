#ifndef _MY_HLS_VIDEO_HOUGH_H_
#define _MY_HLS_VIDEO_HOUGH_H_

namespace my_hls {

template<unsigned int theta,unsigned int rho,typename ACCUMT,int SRC_T,	int ROW,int COL, typename S_T>
void HoughPolar(
        Mat<ROW,COL,SRC_T> &_src,
        ACCUMT 		accum[][(2*(COL+ROW)+1)/rho+2],
        S_T (&sinval)[180/theta],
        S_T (&cosval)[180/theta]
        )
{
    ACCUMT addr[180/theta];
    ACCUMT accbuf[2][180/theta];
//#pragma HLS ARRAY_PARTITION variable=addr complete dim=0
//#pragma HLS ARRAY_PARTITION variable=accbuf complete dim=0
    int rows=_src.rows;
    int cols=_src.cols;
    unsigned int rhoN =(2*(COL+ROW)+1)/ rho;
    unsigned int Half_rhoN=(rhoN-1)/2;
    unsigned int AngleN= 180/theta;
//#pragma HLS ARRAY_PARTITION variable=sinval complete dim=0
//#pragma HLS ARRAY_PARTITION variable=cosval complete dim=0
 loop_init_r: for(int r=0;r<rhoN+2;r++)
    {
    loop_init_n: for(int n=0;n<AngleN+2;n++)
        {
#pragma HLS PIPELINE
            accum[n][r]=0;
        }
    }

 loop_init: for(int n = 0; n < AngleN; n++ )
    {
        addr[n]=0;
        accbuf[0][n]=accum[n+1][Half_rhoN+1];
        sinval[n]=sinval[n]/rho;
        cosval[n]=cosval[n]/rho;
    }
    // stage 1. fill accumulator

 loop_height: for( int i = 0; i < rows; i++ )
    {
    loop_width: for( int  j = 0; j < cols; j++ )
        {
//#pragma HLS PIPELINE
//#pragma HLS DEPENDENCE array inter false
            HLS_TNAME(SRC_T) temp, temp1, temp2;
            _src.data_stream[0]>>temp;
            _src.data_stream[1]>>temp1;
            _src.data_stream[2]>>temp2;
            if(temp!=0&&!(i==0&&j==0))
            {
            loop_angle: for(int n = 0; n < AngleN; n++ )
                {
                    accbuf[1][n]=accbuf[0][n];
                    ACCUMT r=j*cosval[n]+i*sinval[n];
                    r+=Half_rhoN;
                    accbuf[0][n]=accum[n+1][r+1];//if read after write operation then it will have denpendecy problem
                    if(r==addr[n])
                        accbuf[0][n]=accbuf[0][n]+1;

                    accum[n+1][addr[n]+1]=accbuf[1][n]+1;
                    addr[n]=r;
                }
            }

        }
    }
 loop_exit: for(int n = 0; n < AngleN; n++ )
    {
        accum[n+1][addr[n]+1]=accbuf[0][n]+1;
    }
}

#if 1
template<int rho,int ROW,int COL,typename ACCUMT, typename AT, typename RT,int linesMax>
void Sort_insert(
        ACCUMT 	        accum[][(2*(COL+ROW)+1)/rho+2],
        Polar_<AT,RT>   (&lines)[linesMax],
        unsigned int 	threshold,
        unsigned int    theta,
        unsigned  int& 	storenum)
{
    LineBuffer<2,((2*(COL+ROW)+1)/rho+2),ACCUMT>	accum_buf;// used for get local max or min
    Window<3,3,ACCUMT>				accum_win;
    ACCUMT          lines_val[linesMax+1];
    ap_uint<1>      lines_flag[linesMax+1];
    unsigned int rhoN =(2*(COL+ROW)+1)/ rho;
    unsigned int AngleN= 180/theta;
    unsigned int Half_rhoN=(rhoN-1)/2;

//#pragma HLS ARRAY_PARTITION variable=accum_buf complete dim=1
//#pragma HLS ARRAY_PARTITION variable=lines_val complete dim=1
//#pragma HLS ARRAY_PARTITION variable=lines complete dim=0
//#pragma HLS ARRAY_PARTITION variable=accum_win complete dim=0
//#pragma HLS ARRAY_PARTITION variable=lines_flag complete dim=0

 loop_init: for(int i=0;i<linesMax;i++)
    {
        lines[i].angle=0;
        lines[i].rho=0;
        lines_val[i]=0;//
        lines_flag[i]=0;//
    }
        lines_val[linesMax]=0;//
        lines_flag[linesMax]=0;//
    // find the location and sort descent
 loop_n: for(int n=0;n<AngleN+2;n++)
    {
    loop_r: for(int r=0;r<rhoN+2;r++)
        {
//#pragma HLS DEPENDENCE array intra false
//#pragma HLS DEPENDENCE array inter false
//#pragma HLS PIPELINE
            for(int row=2;row>=0;row--)
            {
                for(int col=2;col>=1;col--)
                {
                    accum_win.val[row][col]=accum_win.val[row][col-1];// window col shift
                }
            }

            ACCUMT temp=accum[n][r];

            ACCUMT accum_buf0=accum_buf.val[0][r];

            accum_win.val[2][0]=accum_buf.val[1][r];
            accum_win.val[1][0]=accum_buf0;
            accum_win.val[0][0]=temp;

            accum_buf.val[1][r]=accum_buf0;// buffer shift

            accum_buf.val[0][r]=temp;

            if(r>=2&&r!=Half_rhoN+2&&r<rhoN+2&&n>=2&&n<AngleN+2)
            {
                if((accum_win.val[1][1]>threshold)&&(accum_win.val[1][1]>accum_win.val[1][2])&&(accum_win.val[1][1]>=accum_win.val[1][0])&&(accum_win.val[1][1]>accum_win.val[2][1])&&(accum_win.val[1][1]>=accum_win.val[0][1] ))
                {

                loop_lines: for(int i=linesMax-2;i>=0;i--)
                    {
                        ACCUMT max=lines_val[i];
                        if(accum_win.val[1][1]>max)
                        {
                            lines[i+1].rho=lines[i].rho;
                            lines[i+1].angle=lines[i].angle;
                            lines_val[i+1]=max;
                            lines_flag[i]=0;
                        }
                        else
                        {
                            lines_flag[i]=1;
                        }
                    }
                    int sum=0;
                loop_sum: for(int i=0;i<linesMax;i++)
                        sum+=lines_flag[i];

                    lines[sum].rho=r-1;
                    lines[sum].angle=n-1;
                    lines_val[sum]=accum_win.val[1][1];
                    storenum++;
                }
            }
        }
    }
}
template<typename _AT, typename _RT,typename AT, typename RT,int linesMax>
void inline GetTopLines(
        Polar_<_AT,_RT>         (&_lines)[linesMax],
        Polar_<AT,RT>           (&lines)[linesMax+1],
        unsigned int  		storenum,
        unsigned int 		theta,
        unsigned int 		rho,
        unsigned int            Half_rhoN)
{
    angleAccuracy_T Angle_accuracy=pai*theta/180;
loop_lines: for(int i=0;i<linesMax;i++)
    {
        if(i<storenum)
        {
            RT r=lines[i].rho-1;
            AT n=lines[i].angle-1;
            _RT _rho = (r - Half_rhoN) * rho;
            _AT _angle = n * Angle_accuracy;
            _lines[i].rho=_rho;
            _lines[i].angle=_angle;
        }
        else
        {
                _lines[i].rho=0;
                _lines[i].angle=0;
        }
    }
}
#endif
//static void icvHoughLinesStandard( const CvMat* img, float rho, float theta,
//                       int threshold, CvSeq *lines, int linesMax )--opencv
template<unsigned int theta,unsigned int rho,typename AT,typename RT,typename ACCUMT,int SRC_T,int ROW,int COL,unsigned int linesMax, typename S_T>
void HoughLinesStandard(
        Mat<ROW,COL,SRC_T> 	&_src,
        Polar_<AT,RT> 			(&_lines)[linesMax],
        ACCUMT 			        _accum[][(2*(COL+ROW)+1)/rho+2],
        unsigned int 			_threshold,
        S_T (&sinval)[180/theta],
        S_T (&cosval)[180/theta]
        )
{
    unsigned int storenum=0;
    Polar_<ap_uint<10>,ACCUMT > lines[linesMax+1];
    unsigned int rhoN =(2*(COL+ROW)+1)/ rho;
    unsigned int Half_rhoN=(rhoN-1)/2;

    my_hls::HoughPolar<theta,rho>(_src,_accum, sinval, cosval);
    my_hls::Sort_insert<rho,ROW,COL>(_accum,lines, _threshold,theta,storenum);
    my_hls::GetTopLines(_lines,lines,storenum,theta,rho,Half_rhoN);
}
//CV_IMPL CvSeq*
//cvHoughLines2( CvArr* src_image, void* lineStorage, int method,
//               double rho, double theta, int threshold,
//               double param1, double param2 )---opencv
//
//extern define int accum[theta+2][2*(COL+ROW)+3];
//extern define CvSeq_data lines[linemaxs];

template<unsigned int theta,unsigned int rho,typename AT,typename RT,int SRC_T,int ROW,int COL,unsigned int linesMax>
void HoughLines2(
        Mat<ROW,COL,SRC_T> &	_src,
	Polar_<AT,RT> (&_lines)[linesMax],
	unsigned int 	threshold
        )
{
	static ap_uint<12> _accum[180/theta+2][(2*(COL+ROW)+1)/rho+2];
	//#pragma HLS ARRAY_PARTITION variable=_accum complete dim=1
        ap_fixed<19,2,AP_RND> sinval[180/theta];
        ap_fixed<19,2,AP_RND> cosval[180/theta];
        angleAccuracy_T Angle_accuracy=pai*theta/180;
 loop_init:  for(int i=0;i<180/theta;i++)
        {
            //sinval[i]=::hls::sinf((i*Angle_accuracy));
            sinval[i]=sinf((i*Angle_accuracy));
            //cosval[i]=::hls::cosf(i*Angle_accuracy);
            cosval[i]=cosf(i*Angle_accuracy);
        }
	my_hls::HoughLinesStandard<theta,rho>(_src,_lines,_accum,threshold, sinval,cosval);
}

}
#endif
