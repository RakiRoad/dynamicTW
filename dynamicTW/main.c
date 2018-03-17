#include "m_pd.h"
#include <math.h>
#define SIZE_ARRAY 5


static t_class *dynamicTW_class; //handle for the class

float recording_array[SIZE_ARRAY] = {0};
int arr_position = 0;

int tempArr1[SIZE_ARRAY] = {1, 3, 6, 11, 9};
int tempArr2[SIZE_ARRAY] = {7, 7, 9, 2, 8};
int twoD[SIZE_ARRAY][SIZE_ARRAY] = {0};

typedef struct _leftbottom{
    int left;
    int bottom;
    int diag;
}leftBD; //typedef name

typedef struct _dynamicTW{
    t_object x_obj;

    //following will be placeholder from tutorial
    t_int init_count, current_count;
    t_int mod_A, mod_B; //prob will need

    t_inlet *in_mod_A, *in_mod_B;

    float signal[SIZE_ARRAY];
    int initMatrix[SIZE_ARRAY][SIZE_ARRAY];
    leftBD costValues[SIZE_ARRAY-1][SIZE_ARRAY-1];

}t_dynamicTW; //typedef name



/*======================================================================*/
void dtw_setMods(t_dynamicTW *x, t_floatarg f1, t_floatarg f2){
    x->mod_A = (f1 <= 0)? 1:f1;
    x->mod_B = (f2 <= 0)? 1:f2;
}

void dtw_resetCount(t_dynamicTW *x){
    x->init_count = 0;
    x->current_count = x->init_count;
}
/*======================================================================*/
/*
 1. Create a 2d array for our matrix
 2. fill in the left column and bottom row with our 2 signals
 3. Populate the rest of the matrix
 4. calculate least cost path
*/

void leastCostPath(t_dynamicTW *x){
    int temp, min;
    int result = 0;
    int i = SIZE_ARRAY - 2;
    int j = SIZE_ARRAY - 2;

    post("starting point value left is %d", x->costValues[i][j].left);
    post("starting point value bottom is %d", x->costValues[i][j].bottom);
    post("starting point value diag is %d", x->costValues[i][j].diag);

    while(i >= 0 && j >= 0){
        int checkLeft = x->costValues[i][j].left;
        int checkBottom = x->costValues[i][j].bottom;
        int checkDiagonal = x->costValues[i][j].diag;

        temp = (checkLeft < checkBottom) ? checkLeft : checkBottom;
        min = (checkDiagonal < temp) ? checkDiagonal : temp;

        result += min;
        post("min value is %d", min);

        if(min == checkDiagonal){
            if (i-1 < 0 && j-1 < 0){
                break;
            }
            else if (i-1 < 0){
                j--;
            }
            else if (j-1 < 0){
                i--;
            }
            else{
                i--;
                j--;
            }

            post("changing index to [%d][%d]",j,i);
        }
        else if (min == checkLeft){
            if(j-1 < 0){
                break;
            }
            else{
                j--;
            }
            post("changing index to [%d][%d]",j,i);
        }
        else{
            if(i-1 < 0){
                break;
            }
            else{
                i--;
            }
            post("changing index to [%d][%d]",j,i);
        }

    }

    post("least cost path is %d", result);

}
//Currently testing if I can populate 2d array
//WORKS
void dtw_genMatrix(t_dynamicTW *x){
    int i;
    for(i = 0; i<SIZE_ARRAY; i++){
        x->initMatrix[i][0] = tempArr1[i]; //populate the first column
        x->initMatrix[0][i] = tempArr2[i]; //populate the last row
    }

    /*  //printing left column for signal 1
    int j;
    for (j = 0; j <SIZE_ARRAY;j++){
        post("left column at index %d: %d", j, x->initMatrix[j][0]);
    }

    int k;  //printing bottom values for signal 2
    for(k = 0; k < SIZE_ARRAY; k++){
        post("bottom row at index %d: %d", k, x->initMatrix[0][k]);
    }
    */


    int z, y;
    for(z = 1; z< SIZE_ARRAY; z++){
        for(y=1; y<SIZE_ARRAY; y++){
            int difference = tempArr1[z] - tempArr2[y];
            x->initMatrix[z][y] = (int)pow(difference, 2); //finding the euclidian distance
        }
    }


    //just prints out the values
    int xx, yy;
    for(xx = 1; xx< SIZE_ARRAY; xx++){
        for(yy=1; yy<SIZE_ARRAY; yy++){
            post("at row = %d and column = %d value is: %d", xx, yy, x->initMatrix[xx][yy]);
        }
    }
    post(" ");

    //testing values for left bottom values
    int q, r;
    for(q = 0; q< SIZE_ARRAY-1; q++){
        for(r= 0; r<SIZE_ARRAY-1; r++){
            //no left, bottom, or diagonal values
            if (q == 0 && r ==0){
                x->costValues[q][r].left = 0;
                x->costValues[q][r].bottom = 0;
                x->costValues[q][r].diag = 0;
            }
            //if row is bottom there cant be any bottom values or diagonal so set left values only
            else if (q == 0){
                if (r == 0){
                    x->costValues[q][r].left = 1000; //no possible left values
                }
                else{
                    x->costValues[q][r].left = abs((x->initMatrix[q+1][r+1] - x->initMatrix[q+1][r]));
                }
                x->costValues[q][r].bottom = 1000;
                x->costValues[q][r].diag = 1000;
            }

            //if column is left there cant be any left values or diagonal so set bottom values only
            else if (r == 0){
                if (q == 0){
                    x->costValues[q][r].bottom = 1000;
                }
                else{
                    x->costValues[q][r].bottom = abs((x->initMatrix[q+1][r+1] - x->initMatrix[q][r+1]));
                }
                x->costValues[q][r].left = 1000;
                x->costValues[q][r].diag = 1000;
            }

            //must have left, bottom, and diagonal values
            else{
                x->costValues[q][r].left = abs((x->initMatrix[q+1][r+1] - x->initMatrix[q+1][r]));
                x->costValues[q][r].bottom = abs((x->initMatrix[q+1][r+1] - x->initMatrix[q][r+1]));
                x->costValues[q][r].diag = abs((x->initMatrix[q+1][r+1] - x->initMatrix[q][r]));;
            }

        }
    }

    //testing if values are correct

    int qq, rr;
    for(qq = 0; qq< SIZE_ARRAY-1; qq++){
        for(rr=0; rr<SIZE_ARRAY-1; rr++){
            post("value at row %d and column %d for left is %d: ", qq, rr, x->costValues[qq][rr].left);
            post("value at row %d and column %d for bottom is %d: ", qq, rr, x->costValues[qq][rr].bottom);
            post("value at row %d and column %d for diag is %d: ", qq, rr, x->costValues[qq][rr].diag);
            post(" ");
        }
    }


    leastCostPath(x); //finds least cost path
}

/*what to do when bang is hit*/
void dtw_onBangMsg(t_dynamicTW *x){
    /*
    post("*[dtw ] is set to go");
    if(x->signal[SIZE_ARRAY - 1] == 0){
        post("NONE");
    }
    else{
        post("ARRAY TEST: %f", x->signal[SIZE_ARRAY - 1]);
    }
    */
    dtw_genMatrix(x);
}

void dtw_onSet_A(t_dynamicTW *x, t_floatarg f){  /*function that gets called when an input is received */
    post("Number A: %f sending to array",f);
    //recording_array[0] = f;
                                        /*=======================REVERSE THIS=================================*/
    if (x->signal[SIZE_ARRAY - 1] == 0 && arr_position <= SIZE_ARRAY){ //checks if array is filled. If not then store incoming value to next index
        x->signal[arr_position] = f;
        arr_position++;
    }
    else{                                 //If array is filled shift all values by 1 index and store at beginning of array
        int i;
        for (i = 9; i > 0; i--){
            x->signal[i]=x->signal[i-1];
        }
        x->signal[0] = f;
    }
}

void dtw_onSet_B(t_dynamicTW *x, t_floatarg f){ /*function that gets called when an input is received */
    post("Number B: %f sending to array",f);
    recording_array[0] = f;
}
//initializer for the class
void *dynamicTW_new(t_floatarg f1, t_floatarg f2){ //parenth contains creation arg. temp stuff will replaced with arrays
    t_dynamicTW *x = (t_dynamicTW *)pd_new(dynamicTW_class); //initialize struct of type dtw

    dtw_resetCount(x); //temp
    dtw_setMods(x,f1,f2); //temp

    x->in_mod_A = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ratio_A"));
    x->in_mod_B = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("ratio_B"));
    return (void *)x;
}

void dtw_free(t_dynamicTW *x){
    inlet_free(x->in_mod_A);
    inlet_free(x->in_mod_A);
}


//function to set up the class and call initializer
void dynamicTW_setup(void){
    /*class_new(t_symbol *name, t_newmethod newmethod,
    t_method freemethod, size_t size, int flags, t_atomtype arg1, ...); */
    dynamicTW_class = class_new(gensym("dynamicTW"), //defines the symbol in puredata
                          (t_newmethod)dynamicTW_new, //inializing method
                          0,
                          sizeof(t_dynamicTW),
                          CLASS_DEFAULT,//makes the box
                          A_DEFFLOAT,
                          A_DEFFLOAT,
                          0);
    class_addbang(dynamicTW_class, (t_method)dtw_onBangMsg);
    class_addmethod(dynamicTW_class,
                    (t_method)dtw_onSet_A,
                    gensym("ratio_A"),
                    A_DEFFLOAT,
                    0);
    class_addmethod(dynamicTW_class,
                (t_method)dtw_onSet_B,
                gensym("ratio_B"),
                A_DEFFLOAT,
                0);
}
