

/*
	時空間並列回路モデル2D
	  
	このプログラムは，Pspice A/Dを用いてシミュレーションを行うために必要な，ネットリスト,.ALIASES，.prbを作成します。
	素子とノードの記述以外はPspiceで生成されたファイルと同じ文章を書いています．
	ノード番号の変わり方(つながり方)は以下のとおり

	I_I1	N2	0
	R_Rm1	N1	0
	R_Rp1   N1  N2
	R_Rs1	N1	N3
	C_Cs1	N1	N3

	I_I2	N4	0
	R_Rm2	N3	0
	R_Rp2   N3  N4
	R_Rs2	N3	N5
	C_Cs2	N3	N5

	I_I3	N6	0	
	R_Rm3	N5	0
	R_Rp3   N5  N6	
	R_Rs3	N5	N7
	C_Cs3	N5	N7

*/

#define ANSI
#include <math.h>
#include <stdio.h> 
#include <stdlib.h>
#include<string.h>
#include "nrutil.h" 
#include<time.h>//時間を用いる
#include <direct.h>//フォルダを作成す

void itoa(int val, char *s);

int main(){

////////////////////条件設定　ここから//////////////////////////////////////////////////////////////////////////////
	//回路サイズを指定．ここでは入力電圧のpixel × pixel を指定
	int sosi_max=129;
	char memo[64]="FFT";

	//パラメータを指定
	char Rm_parameter[256]="30k";
	char Rg_parameter[256]="30k";
	char Rs_parameter[256]="0.5k";
	char Rc_parameter[256]="100k";
	char Cs_parameter[256]="10p";

	int x,y;

	//matrix
	int **input1=imatrix(1,1000,1,1000);
	int **input2=imatrix(1,1000,1,1000);
	int **NV=imatrix(1,1000,1,1000);
	int **nVR=imatrix(1,1000,1,1000);
	int **nVU=imatrix(1,1000,1,1000);

	for(y=1;y<=1000;y=y+1){
		for(x=1;x<=1000;x=x+1){

			input1[x][y]=0;
			input2[x][y]=0;
			NV[x][y]=0;
			nVR[x][y]=0;
			nVU[x][y]=0;
		}
	}

	//入力電圧(印加時間）を指定
	/*
	for(y=1;y<=sosi_max;++y){
		
		for(x=1;x<=sosi_max;++x){
			if(y>8){
				if(x<8){
	input1[x][y]=3;//ここはint型でしか指定できないため，小数点を表示するには修正が必要
	input2[y][y]=3;//ここはint型でしか指定できないため，小数点を表示するには修正が必要
				}
			}
	}
	}*/
	
	for(y=1;y<=sosi_max;++y){
	input1[65][y]=3;//ここはint型でしか指定できないため，小数点を表示するには修正が必要
	input2[65][y]=3;//ここはint型でしか指定できないため，小数点を表示するには修正が必要
	}
	//input1[16][16]=3;
	//input2[16][16]=3;

////////////////////////////条件設定　ここまで//////////////////////////////////////////////////////////////////////////

	char *math_name1_s = "SCHEMATIC1.net";				//出力結果のファイル名の指定
	char *math_name2_s = "SCHEMATIC1.ALS";
	char *math_name3_s ="計測用";
	char *math_name4_s;

	char math_name1[64];							//netリスト						
	char math_name2[64];							//ALS
	char math_name3[64];							//観測用
	char math_name4[256];

	int i;

	int parameter_num=1;
	int parameter_num_c=1;
	int palameter_num_c_flag=0;
	int parameter_num_V=1;

	

	int measurement_point;					//0,45,90,135,180,225,270,315を指定

	int count_blue;
	char simulation_profile[128];
	char ALIASES[128]="CN @TEST2.SCHEMATIC1(sch_1)";
	FILE *fp;

////////////////////////////フォルダ作成　ここから/////////////////////////////////////////////////////////
	//実行日時の表示
	time_t now = time(NULL);
    struct tm *pnow = localtime(&now);
	char date[128]="";
	sprintf(date,"%2d/%2d/%2d - %d:%d:%d\n",pnow->tm_year+1900,pnow->tm_mon + 1,pnow->tm_mday,pnow->tm_hour,pnow->tm_min,pnow->tm_sec);
	printf(date);

	//結果を保存するフォルダの生成
	//フォルダ名は実行日時になる
	char date_directory[128];
	sprintf(date_directory,"..\\result_pen\\%2d-%02d%02d-%02d%02d%02d\\",pnow->tm_year+1900,pnow->tm_mon + 1,pnow->tm_mday,pnow->tm_hour,pnow->tm_min,pnow->tm_sec);
	 if(_mkdir(date_directory)==0){
        printf("フォルダ %s を作成しました\n",date_directory);
    }else{
        printf("フォルダ作成に失敗しました。\n");
    }

	 sprintf(simulation_profile,"4方向回路ネットリスト作成プログラム\n素子数%d×%d\nRm=%s,Rg=%s,Rs=%s,Rc=%s,Cs=%s\n",sosi_max,sosi_max,Rm_parameter,Rg_parameter,Rs_parameter,Rc_parameter,Cs_parameter);
	 printf("%s",simulation_profile);

	strcpy(math_name1, date_directory);
    strcat(math_name1, math_name1_s);
	strcpy(math_name2, date_directory);
    strcat(math_name2, math_name2_s);

/////////////////////////フォルダ作成 ここまで///////////////////////////////////////////////////////////////////////
	
////////////////////////////////netlist（回路データ）ここから///////////////////////////////////////////////////////

/////////////ファイルオープン///////////////////////////////////////////////////////////

	if((fp=fopen(math_name1,"w"))==NULL){
		printf("SCHEMATIC1.netがオープンできません．\n");
		exit(1);
	}

	fprintf(fp,"* source TEST \n\n");			//ファイル名?一行目


///////////////////////Rsの和////////////////////////////////////////////////////////////////

	int N1=8+5*(sosi_max-1)+(5+3*(sosi_max-1))*(sosi_max-1);	//Rsにつなぐノードの数 = この次からRs,Csをつなぎ始める
	int N2=N1+4*sosi_max*sosi_max;									//N1 + Cs,Rcにつなぐノードの数 = この次からRg,Rm,Vをつなぎ始める
	int i2=N1+1;
	int i3=N2+1;
	

	count_blue=1;
	i=1;

	for(y=1;y<=sosi_max;++y){
		for(x=1;x<=sosi_max;++x){

			if(x<sosi_max && y<sosi_max){

//////////y=1 ここから/////////////////////////////////////

			if(y==1){

//////////y=1,x=1 ここから/////////////////////////////////////
				if(x==1){
				for(count_blue=1;count_blue<=8;++count_blue){
					if(count_blue==8){
								
						fprintf(fp,"R_Rs%d	N%d N1 %s\n",parameter_num, i, Rs_parameter);
						fprintf(fp,"R_Rc%d	N%d N1 %s\n",parameter_num_c,i2,Rc_parameter);
						
					}else{
					
					fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);
				}
	
					if(palameter_num_c_flag==0){
						palameter_num_c_flag=1;
					}else{
						
						fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c ,i-1, i2, Cs_parameter);
						if(count_blue!=8)fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c,i2, i+1, Rc_parameter);
						palameter_num_c_flag=0;
						++parameter_num_c;
						++i2;
					}

					
						if(count_blue==1)NV[1][1]=i;
						if(count_blue==3)NV[1][2]=i;
						if(count_blue==5)NV[2][2]=i;
						if(count_blue==7)NV[x+1][y]=i;
					
						if(count_blue==4)nVU[1][1]=i;
						if(count_blue==6)nVR[1][1]=i;
						
						++i;
					
					++parameter_num;

					
					//++parameter_num_c;
					
				}

//////////y=1,x=1 ここまで/////////////////////////////////////
				}else{
//////////y=1,x>1 ここから/////////////////////////////////////

					for(count_blue=1;count_blue<=8;++count_blue){
						if(count_blue==1){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, NV[x][y], nVR[x-1][y], Rs_parameter);
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, NV[x][y], i2, Cs_parameter);
						}
						if(count_blue==2){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, nVR[x-1][y], NV[x][y+1], Rs_parameter);
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, NV[x][y+1], Rc_parameter);
							++i2;
						}
						if(count_blue==3){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, NV[x][y+1], i, Rs_parameter);
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, NV[x][y+1], i2, Cs_parameter);
							
						}
						if(count_blue==4){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);nVU[x][y]=i;
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, i+1, Rc_parameter);
							++i;
							++i2;
						}
						if(count_blue==5){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);NV[x+1][y+1]=i;
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, i, i2, Cs_parameter);
							++i;
							
						}
						
						
						if(count_blue==6){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);nVR[x][y]=i;
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, i+1, Rc_parameter);
							++i;
							++i2;
						}
						if(count_blue==7){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);NV[x+1][y]=i;
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, i, i2, Cs_parameter);

							
							++i;
						}
						
						if(count_blue==8){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, NV[x][y], Rs_parameter);
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, NV[x][y], Rc_parameter);
							++i;
							++i2;
						}
						
						++parameter_num;
						if(palameter_num_c_flag==0){palameter_num_c_flag=1;}
						else{palameter_num_c_flag=0;++parameter_num_c;}
					
					}
			}
//////////y=1,x>1 ここまで/////////////////////////////////////
			}
//////////y=1 ここまで/////////////////////////////////////

//////////y>1 ここから/////////////////////////////////////
			if(y>1){

//////////y>1,x=1 ここから/////////////////////////////////////
				if(x==1){
					for(count_blue=1;count_blue<=8;++count_blue){

						if(count_blue==1){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, NV[x][y], i, Rs_parameter);
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, NV[x][y], i2, Cs_parameter);
						}
						
						

						if(count_blue==2){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, i+1, Rc_parameter);
							++i;
							++i2;
						}
						if(count_blue==3){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);NV[x][y+1]=i;
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, i, i2, Cs_parameter);
							++i;
						}
						if(count_blue==4){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);nVU[x][y]=i;
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, i+1, Rc_parameter);
							++i;
							++i2;
						}
						if(count_blue==5){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);NV[x+1][y+1]=i;
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, i, i2, Cs_parameter);
							++i;
							
						}
						if(count_blue==6){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, NV[x+1][y], Rs_parameter);nVR[x][y]=i;
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, NV[x+1][y], Rc_parameter);
							++i;
							++i2;
						}
						if(count_blue==7){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, NV[x+1][y],nVU[x][y-1], Rs_parameter);
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, NV[x+1][y], i2, Cs_parameter);
							
							
						}
						if(count_blue==8){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, nVU[x][y-1], NV[x][y], Rs_parameter);
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, NV[x][y], Rc_parameter);
							++i2;
							
						}
						
	
						//if(count_blue==5)NV[x][y+1]=i;
						//if(count_blue==9)NV[x+1][y+1]=i;
					
						//if(count_blue==7)nVU[x][y]=i;
						//if(count_blue==11)nVR[x][y]=i;
						
					
					
					++parameter_num;
					if(palameter_num_c_flag==0){palameter_num_c_flag=1;}
					else{palameter_num_c_flag=0;++parameter_num_c;}
////////////////////////////////////////////////////////////////////////////////
					
				}
//////////y>1,x=1 ここまで/////////////////////////////////////
				}else{
					

//////////y>1,x>1 ここから/////////////////////////////////////

					for(count_blue=1;count_blue<=8;++count_blue){

						if(count_blue==1){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, NV[x][y], nVR[x-1][y], Rs_parameter);
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, NV[x][y], i2, Cs_parameter);
						}
						if(count_blue==2){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, nVR[x-1][y], NV[x][y+1], Rs_parameter);
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, NV[x][y+1], Rc_parameter);
							++i2;
							
						}
						if(count_blue==3){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, NV[x][y+1], i, Rs_parameter);
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, NV[x][y+1], i2, Cs_parameter);
							
						}
						if(count_blue==4){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, i+1, Rs_parameter);nVU[x][y]=i;
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, i+1, Rc_parameter);
							++i;
							++i2;
						}
						if(count_blue==5){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num,i, i+1, Rs_parameter);NV[x+1][y+1]=i;
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, i, i2, Cs_parameter);
							++i;
							
						}
						
						
						if(count_blue==6){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, i, NV[x+1][y], Rs_parameter);nVR[x][y]=i;
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, NV[x+1][y], Rc_parameter);
							++i;
							++i2;
						}
						if(count_blue==7){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, NV[x+1][y], nVU[x][y-1], Rs_parameter);nVU[x][y]=i;
							fprintf(fp,"C_Cs%d	N%d N%d %s\n",parameter_num_c, NV[x+1][y], i2, Cs_parameter);
						
							
						}
						if(count_blue==8){
							fprintf(fp,"R_Rs%d	N%d N%d %s\n",parameter_num, nVU[x][y-1], NV[x][y], Rs_parameter);
							fprintf(fp,"R_Rc%d	N%d N%d %s\n",parameter_num_c, i2, NV[x][y], Rc_parameter);
							++i2;
							
						}
						
						
						//if(count_blue==9)NV[x+1][y+1]=i;
						//if(count_blue==13)NV[x+1][y]=i;

						//if(count_blue==7)nVU[x][y]=i;
						//if(count_blue==11)nVR[x][y]=i;

						
						++parameter_num;
						if(palameter_num_c_flag==0){palameter_num_c_flag=1;}
					else{palameter_num_c_flag=0;++parameter_num_c;}
					}
				}

			}
			}
			fprintf(fp,"R_Rm%d	N%d N%d %s\n",parameter_num_V, NV[x][y], i3, Rm_parameter);
			fprintf(fp,"V_V%d	N%d 0	pulse(0V %d.%dV 0us 0.1ns 0.1ns 3.2us 20us)\n", parameter_num_V, i3,input1[x][y],input2[x][y]);
			++i3;
			fprintf(fp,"R_Rg%d	N%d 0 %s\n",parameter_num_V, NV[x][y], Rg_parameter);
			++i3;
			++parameter_num_V;



			
			
			}//xの占め
		}
//////////y>1,x>1 ここまで/////////////////////////////////////

	
	fclose(fp);
	

////////////////////////////////netlist（回路データ）終わり///////////////////////////////////////////////////////


//////////////////////////////////////ALIASES　ここから　基本はnetlistと同じとする//////////////////////////////////////////////////////

	if((fp=fopen(math_name2,"w"))==NULL){
		printf("SCHEMATIC1.ALSがオープンできません．\n");
		exit(1);
	}

	fprintf(fp,".ALIASES \n\n");			//一行目




	//matrix初期化
	for(y=1;y<=1000;y=y+1){
		for(x=1;x<=1000;x=x+1){

			input1[x][y]=0;
			input2[x][y]=0;
			NV[x][y]=0;
			nVR[x][y]=0;
			nVU[x][y]=0;

		}
	}

	parameter_num=1;
	parameter_num_c=1;
	palameter_num_c_flag=0;
	parameter_num_V=1;

	i=1;
	i2=N1+1;
	i3=N2+1;

for(y=1;y<=sosi_max;++y){
		for(x=1;x<=sosi_max;++x){
			if(x<sosi_max && y<sosi_max){

//////////ALIASES　y=1 ここから/////////////////////////////////////

			if(y==1){

//////////ALIASES　y=1,x=1 ここから/////////////////////////////////////
				if(x==1){
				for(count_blue=1;count_blue<=8;++count_blue){
					if(count_blue==8){
								
						fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N1 )%s\n",parameter_num,parameter_num, i, ALIASES);
						fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N1 )%s\n",parameter_num_c,parameter_num_c,i2, ALIASES);
					}else{
					
					fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1,  ALIASES);
				}
	
					if(palameter_num_c_flag==0){
						palameter_num_c_flag=1;
					}else{
						
						fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c,i-1, i2, ALIASES);
						if(count_blue!=8)fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c,i2, i+1, ALIASES);
						palameter_num_c_flag=0;
						++parameter_num_c;
						++i2;
					}

					
						if(count_blue==1)NV[1][1]=i;
						if(count_blue==3)NV[1][2]=i;
						if(count_blue==5)NV[2][2]=i;
						if(count_blue==7)NV[x+1][y]=i;
					
						if(count_blue==4)nVU[1][1]=i;
						if(count_blue==6)nVR[1][1]=i;
						
						++i;
					
					++parameter_num;

					
					//++parameter_num_c;
					
				}
				
//////////ALIASES　y=1,x=1 ここまで/////////////////////////////////////
				}else{
//////////ALIASES　y=1,x>1 ここから/////////////////////////////////////

					for(count_blue=1;count_blue<=8;++count_blue){
						if(count_blue==1){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, NV[x][y], nVR[x-1][y], ALIASES);
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, NV[x][y], i2, ALIASES);
						}
						if(count_blue==2){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, nVR[x-1][y], NV[x][y+1], ALIASES);
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, NV[x][y+1], ALIASES);
							++i2;
						}
						if(count_blue==3){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, NV[x][y+1], i, ALIASES);
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, NV[x][y+1], i2, ALIASES);
							
						}
						if(count_blue==4){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);nVU[x][y]=i;
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, i+1, ALIASES);
							++i;
							++i2;
						}
						if(count_blue==5){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);NV[x+1][y+1]=i;
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i, i2, ALIASES);
							++i;
							
						}
						

						if(count_blue==6){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);nVR[x][y]=i;
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, i+1, ALIASES);
							++i;
							++i2;
						}
						if(count_blue==7){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);NV[x+1][y]=i;
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i, i2, ALIASES);

							
							++i;
						}
						
						if(count_blue==8){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, NV[x][y], ALIASES);
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, NV[x][y], ALIASES);
							++i;
							++i2;
						}
						
						++parameter_num;
						if(palameter_num_c_flag==0){palameter_num_c_flag=1;}
						else{palameter_num_c_flag=0;++parameter_num_c;}
					
					}
			}
//////////ALIASES　y=1,x>1 ここまで/////////////////////////////////////
			}
//////////ALIASES　y=1 ここまで/////////////////////////////////////

//////////ALIASES　y>1 ここから/////////////////////////////////////
			if(y>1){

//////////ALIASES　y>1,x=1 ここから/////////////////////////////////////
				if(x==1){
					for(count_blue=1;count_blue<=8;++count_blue){

						if(count_blue==1){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, NV[x][y], i, ALIASES);
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, NV[x][y], i2, ALIASES);
						}
						
						

						if(count_blue==2){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, i+1, ALIASES);
							++i;
							++i2;
						}
						if(count_blue==3){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);NV[x][y+1]=i;
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i, i2, ALIASES);
							++i;
						}
						if(count_blue==4){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);nVU[x][y]=i;
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, i+1, ALIASES);
							++i;
							++i2;
						}
						if(count_blue==5){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);NV[x+1][y+1]=i;
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i, i2, ALIASES);
							++i;
							
						}
						if(count_blue==6){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, NV[x+1][y], ALIASES);nVR[x][y]=i;
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, NV[x+1][y], ALIASES);
							++i;
							++i2;
						}
						if(count_blue==7){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, NV[x+1][y],nVU[x][y-1], ALIASES);
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, NV[x+1][y], i2, ALIASES);
							
							
						}
						if(count_blue==8){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, nVU[x][y-1], NV[x][y], ALIASES);
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, NV[x][y], ALIASES);
							++i2;
							
						}
						
	
						//if(count_blue==5)NV[x][y+1]=i;
						//if(count_blue==9)NV[x+1][y+1]=i;
					
						//if(count_blue==7)nVU[x][y]=i;
						//if(count_blue==11)nVR[x][y]=i;
						
					
					
					++parameter_num;
					if(palameter_num_c_flag==0){palameter_num_c_flag=1;}
					else{palameter_num_c_flag=0;++parameter_num_c;}
////////////////////////////////////////////////////////////////////////////////
					
				}
//////////ALIASES　y>1,x=1 ここまで/////////////////////////////////////
				}else{
					

//////////ALIASES　y>1,x>1 ここから/////////////////////////////////////

					for(count_blue=1;count_blue<=8;++count_blue){

						if(count_blue==1){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, NV[x][y], nVR[x-1][y], ALIASES);
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, NV[x][y], i2, ALIASES);
						}
						if(count_blue==2){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, nVR[x-1][y], NV[x][y+1], ALIASES);
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, NV[x][y+1], ALIASES);
							++i2;
							
						}
						if(count_blue==3){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, NV[x][y+1], i, ALIASES);
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, NV[x][y+1], i2, ALIASES);
							
						}
						if(count_blue==4){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, i+1, ALIASES);nVU[x][y]=i;
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, i+1, ALIASES);
							++i;
							++i2;
						}
						if(count_blue==5){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num,i, i+1, ALIASES);NV[x+1][y+1]=i;
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i, i2, ALIASES);
							++i;
							
						}
						
						
						if(count_blue==6){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, i, NV[x+1][y], ALIASES);nVR[x][y]=i;
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, NV[x+1][y], ALIASES);
							++i;
							++i2;
						}
						if(count_blue==7){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, NV[x+1][y], nVU[x][y-1], ALIASES);nVU[x][y]=i;
							fprintf(fp,"C_Cs%d	Cs%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, NV[x+1][y], i2, ALIASES);
						
							
						}
						if(count_blue==8){
							fprintf(fp,"R_Rs%d	Rs%d(1=N%d 2=N%d )%s\n",parameter_num,parameter_num, nVU[x][y-1], NV[x][y], ALIASES);
							fprintf(fp,"R_Rc%d	Rc%d(1=N%d 2=N%d )%s\n",parameter_num_c,parameter_num_c, i2, NV[x][y], ALIASES);
							++i2;
							
						}
						
						
						//if(count_blue==9)NV[x+1][y+1]=i;
						//if(count_blue==13)NV[x+1][y]=i;

						//if(count_blue==7)nVU[x][y]=i;
						//if(count_blue==11)nVR[x][y]=i;

						
						++parameter_num;
						if(palameter_num_c_flag==0){palameter_num_c_flag=1;}
					else{palameter_num_c_flag=0;++parameter_num_c;}
					}
				}

			}
			}
			
			fprintf(fp,"R_Rm%d	Rm%d(1=N%d 2=N%d )%s\n",parameter_num_V,parameter_num_V, NV[x][y], i3, ALIASES);
			fprintf(fp,"V_V%d	V%d(+-=N%d -=0	)%s\n",parameter_num_V,parameter_num_V, i3, ALIASES);
			++i3;
			fprintf(fp,"R_Rg%d	Rg%d(1=N%d 2=0 )%s\n",parameter_num_V,parameter_num_V, NV[x][y], ALIASES);
			++i3;
			++parameter_num_V;

			

			
			
			}//xの占め
		}
//////////ALIASES　y>1,x>1 ここまで/////////////////////////////////////


//////////ALIASES y>1,x>1 ここまで/////////////////////////////////////

	fprintf(fp,".ENDALIASES \n");			//末尾
	fclose(fp);

//////////////////////////////////////ALIASES終わり//////////////////////////////////////////////////////


////////////////////////////////////////////計測用.txt/////////////////////////////////////////////

	int count_measure;
	int measurement_number;
	int count_sum;

	for(count_measure=1;count_measure<=4;++count_measure){

		count_sum=0;
		measurement_point=90*(count_measure-1);

		if(measurement_point==0)measurement_number=3;
		if(measurement_point==90)measurement_number=4;
		if(measurement_point==180)measurement_number=1;
		if(measurement_point==270)measurement_number=2;

		sprintf(math_name3,"%s%s%d.txt",date_directory,math_name3_s,measurement_point);

		if((fp=fopen(math_name3,"w"))==NULL){
			printf("計測用.txtがオープンできません．\n");
			exit(1);
		}

/*		int count_ani=0;
	for(y=1;y<sosi_max;++y){
		
		for(x=1;x<sosi_max;++x){	
			int i4=measurement_number+count_sum;			//6が0°ここから反時計回りに7
			fprintf(fp,"V(Rc%d:1) ", i4);
			count_sum+=4;
			++count_ani;
			if(count_ani==400){fprintf(fp,"\n");count_ani=0;}
			if(count_ani==100 || count_ani==200 || count_ani==300){fprintf(fp,"\t");}
		}
		//fprintf(fp,"\n");		//素子一行ごとに改行
	}
	fclose(fp);
	}*/

	int countokan=0;
	for(y=1;y<sosi_max;++y){
		++countokan;
		for(x=1;x<sosi_max;++x){	
			int i4=measurement_number+count_sum;			//6が0°ここから反時計回りに7
			
			fprintf(fp,"V(Rc%d:1) ", i4);
			count_sum+=4;
		}
		fprintf(fp,"\n");
		if(countokan==63){fprintf(fp,"\n\n\n");}	//素子一行ごとに改行
		if(countokan==64){fprintf(fp,"\n\n\n");}	//素子一行ごとに改行
		//if(countokan==6){fprintf(fp,"\n");countokan=0;}	//素子一行ごとに改行
		fprintf(fp,"\n");
		
	}
	fclose(fp);
	}
	
////////////////////////////////////////////計測用終わり/////////////////////////////////////////////

/////////////////////////////////////////logファイルの作成/////////////////////////////////////////////
	sprintf(math_name4,"%s4dire_RmRg%sRs%sRc%sCs%s_%s_netlist.txt",date_directory,Rm_parameter,Rs_parameter,Rc_parameter,Cs_parameter,memo);
	if((fp=fopen(math_name4,"w"))==NULL){
			printf("log.txtがオープンできません．\n");
			exit(1);
		}

	fprintf(fp,"4dire_RmRg%sRs%sRc%sCs%s_netlist",Rm_parameter,Rs_parameter,Rc_parameter,Rc_parameter);

	fclose(fp);
	
	//配列の解法

	free_imatrix(input1, 1,1000,1,1000);
	free_imatrix(input2, 1,1000,1,1000);
	free_imatrix(NV, 1,1000,1,1000);
	free_imatrix(nVR, 1,1000,1,1000);
	free_imatrix(nVU, 1,1000,1,1000);

	

	return 0;
}
