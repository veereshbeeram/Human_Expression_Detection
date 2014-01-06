/*
 * main.c
 *
 *  Created on: Apr 7, 2011
 *      Author: Veeresh Beeram
 *
 *
 *  ## Dept Of Information Technology - NITK, surathkal##
 *  #################### Major Project ##################
 *  ############# Human Expression Recognition###########

    Veeresh Beeram


 *
 */


//preprocessing include directives


#include <cv.h>
#include <highgui.h>
#include <stdio.h>
#include <stdlib.h>



// preprocessing constants
#define MIN_ARGS 4
#define ASK_USER 1
#define VERBOSE_DISPLAY_TIMER 2
#define DISPLAY_WIDTH 500
#define DISPLAY_HEIGHT 400


/*
 * General Program Flow:
 *
 * 		Global Data structures:
 * 				Input
 * 					upto 10 images. 2 in case of static image analysis. Upto 10 incase of camera/video key frame
 * 					extraction. we are storing original,final and masks for each of the images.
 * 					A Live Image structure.
 * 				Output
 * 					Upto 10 datastructures for 10 images
 *
 * 				FrameCount
 * 					to keep track of how many frames/images are considered for processing
 *
 * 				In all above cases index 0 has the base image and its value
 *
 * 				A mouse mark data structure to temporarily store the input from mouse. and associated count to keep track of how many points
 * 				were marked.
 *
 *
 * 		Program Flow:
 *
 * 				Main calls frameExtract
 * 					frameExtract is supposed to populate the Image structures. these are the
 * 					images that go in for Image processing. It returns to main with success/not
 *
 * 				On success Main calls imageProcess
 * 					This is the stub function. It calls each step of image process. If not
 * 					successful, will prompt for user input of point, if critical does the
 * 					required exit mechanism. Also populates the corresponding field of o/p DS.
 * 					Then if verbose set,shows the user the intermediate result.
 * 					Returns over all success/failure
 *
 *				On success Main calls output function
 *					Output handles the presentation and printing of output in reqd format, from
 *					the data in data structures
 *
 *		Naming Convention:
 *
 *				All functions
 *					Upper camel case
 *
 *				All local variables
 *					lower camel case
 *
 *				All global variables
 *					GBL_<varName> varName has to be in lower camel case
 *
 *				Declare all variables at the start of the function/ start of the C source file
 */

// Global data structures

// Images

	IplImage *GBL_originalImg[10],*GBL_finalImg[10],*GBL_liveImg,*GBL_maskImg[10];

// Output structure

	typedef struct OUTPUT{

		int imgWidth,imgHeight;
		int isFace;
		int isError;
		CvRect face; // CvRect for face ROI
		CvRect eyes[2]; // CvRect[2] for eyes
		CvRect nose; // CvRect for nose
		CvRect mouth; // CvRect for mouth
		CvPoint lbrow[2]; //left eye brow
		CvPoint rbrow[2]; //right eye brow
		CvPoint cheek[2]; // 0 -left; 1-right
		int lipCount;
	}OutStruct;

	OutStruct GBL_output[10];

	int GBL_frameCount=0,GBL_verbose=0,GBL_askUser=ASK_USER,GBL_inType=0;
	char *GBL_basefile, *GBL_emotionfile, *GBL_videofile;

	CvPoint GBL_mouseMark[20];
	int GBL_mouseMarkCount;
	int GBL_eyebrow[3];
	int GBL_lipEdgex,GBL_lipEdgey;

// function declarations

	int exitFunction(int);
	int copyRect(CvRect *src, CvRect *dest);
	void on_mouse(int ,int ,int ,int,void*);

	//frame extract
		int frameExtractImage();
		int frameExtractVideo();
		int frameExtractCamera();
	// image process
		int imageProcess();
		void faceDetect(int);
		void eyeMouthNoseDetect(int);
		int faceSegment(int,int,int,int);
		void fineCleanHair(int);
			void connectedRecursive(IplImage*,int,int,int);
			void connectedRecursiveCount(IplImage* ,int,int,int,int,int,int);
		void cheekDetect(int);
		void lipContour(int);
            void cleanBeard(int);

     // output
            void outputProcess();


// main function
/*
 * Parameters into main function and what they mean
 * 		params: verbose,inputtype,askuser,[file1],[file2]
 *
 *			basic intc should be atleast 4.
 *
 * 			verbose: should I show the output at each stage. 0 means don't show
 *
 * 			inputtype
 * 				0 - 2 static images, file1 is base image, file2 is emotion image. intc should be basicVal+2
 *
 * 				1 - a video file, file1 is the path to file. intc  should be basicVal+1
 *
 * 				2 - camera. No input files. intc is basicVal.
 *
 * 			askuser
 * 				this is for further extension. If set to zero then all failures to detect features will result in soft error
 * 				which will be logged instead of asking user. This is to record accuracy of image processing functions.
 * 				DEFAULT should be 1
 */

	int main(int argc, char* argv[]){

		// main entry point of program
		// check args, sets args into vars, calls frameExtract, calls imageProcess

		printf("CALL - main()\n");

		//local variables
		int retVal=0;
		FILE *test;

		//loop variables


		//-------------- code logic -----------------------------

		// program args assertion
		if(argc < MIN_ARGS){
			fprintf(stderr,"ERROR: Need input parameters:<verbose,inputtype,askuser>");
			return -1;
		}

		GBL_verbose = atoi(argv[1]);
		GBL_inType = atoi(argv[2]);
		GBL_askUser = atoi(argv[3]);
		/*
		 * code to find out the working directory of the program
		 * CODE :
		 *		char *pwd = malloc(500);
		 *		getcwd(pwd,500);
		 *		printf("%s\n",pwd);
		 *
		 */


		switch (GBL_inType) {
		// checking args based on type of analysis
		// 0 - static images ; 1 - video feed ; 2 - camera feed
			case 0:
				if(argc<MIN_ARGS+2){
					fprintf(stderr,"ERROR: Need <base image, emotion image> for static image analysis");
					return -1;
				}
				test = fopen(argv[4],"r");
				if(!test){
					fprintf(stderr,"ERROR_FILE_NOT_FOUND: base image file for static image analysis not found");
					return exitFunction(-1);
				}
				else{
					fclose(test);
					GBL_basefile = malloc(500);
					strcpy(GBL_basefile,argv[4]);
				}
				test = fopen(argv[5],"r");
				if(!test){
					fprintf(stderr,"ERROR_FILE_NOT_FOUND: emotion image file for static image analysis not found");
					return exitFunction(-1);
				}
				else{
					fclose(test);
					GBL_emotionfile = malloc(500);
					strcpy(GBL_emotionfile,argv[5]);

				}

				break;
			case 1:
				if(argc<MIN_ARGS+1){
					fprintf(stderr,"ERROR: Need <video file> for video feed analysis");
					return exitFunction(-1);
				}
				test = fopen(argv[4],"r");
				if(!test){
					fprintf(stderr,"ERROR_FILE_NOT_FOUND:video file for video feed analysis not found");
					return exitFunction(-1);
				}
				else{
					fclose(test);
					GBL_videofile = malloc(500);
					strcpy(GBL_videofile,argv[4]);

				}

				break;
			default:
				break;
		}

		// All parameters seem to have been passed. Now for default value setting



		printf(" DEBUG - main() ::All parameters successfully passed now proceeding to frame extraction\n");
		switch (GBL_inType) {
			case 0:
				retVal = frameExtractImage();
				if(retVal!=0){
					printf("DEBUG - main():: frameExtractImage() call failed\n");
				}
				else{
					printf("DEBUG - main():: frameExtractImage() call passed\n");
					retVal = imageProcess();

					// make sense of GBL_output data and produce MLP feedable format
					outputProcess();
				}
				break;
			case 1:
				retVal = frameExtractVideo();
				if(retVal!=0){
					printf("DEBUG - main():: frameExtractVideo() call failed\n");
				}
				else{
					printf("DEBUG - main():: frameExtractVideo() call passed\n");
					//TODO call image processing functions. Expected to be same as above.
				}

				break;
			case 2:
				retVal = frameExtractCamera();
				if(retVal!=0){
					printf("DEBUG - main():: frameExtractVideo() call failed\n");
				}
				else{
					printf("DEBUG - main():: frameExtractVideo() call passed\n");
					//TODO call image processing functions. Expected to be same as above.
				}

				break;
			default:
				break;
		}

        int abc = system("./out");
        printf("%d\n",abc);
        switch (abc){
            case 0:
                printf("No emotion\n");
                break;
            case 1:
                printf("smile\n");
                break;
            case 2:
                printf("surprise\n");
                break;
            case 3:
                printf("frown\n");
                break;

        }
		//default return value
		return exitFunction(0);

	}

	int exitFunction(int retval){

		//check if any of global pointers have been set. In case they have been, free them

		printf("CALL - exitFunction()\n");
		// TODO throwing some core dump.. since safe exit is low priority do it later
		return retval;

		//local vars
		//loop vars
		int loopi=0;
		//-------------- code logic -----------------------------

		for(loopi=0;loopi<10;loopi++){
			if(GBL_originalImg[loopi]){
				free(GBL_originalImg[loopi]);
			}
			if(GBL_finalImg[loopi]){
				free(GBL_finalImg[loopi]);
			}
			if(GBL_maskImg[loopi]){
					free(GBL_maskImg[loopi]);
			}


		}

		if(GBL_liveImg){
				free(GBL_liveImg);
		}


		if(GBL_basefile){
				free(GBL_basefile);
		}

		if(GBL_emotionfile){
				free(GBL_emotionfile);
		}

		if(GBL_videofile){
				free(GBL_videofile);
		}



		return retval;

	}

	void on_mouse(int event,int x,int y,int flags,void* param){

		// generic mouse handler.
		// sets x,y into mousemark global

		// printf("CALL - on_mouse()\n"); // commented due to overload

		if(event==CV_EVENT_LBUTTONDOWN)
		{
			GBL_mouseMark[GBL_mouseMarkCount].x = x;
			GBL_mouseMark[GBL_mouseMarkCount].y = y;
			GBL_mouseMarkCount++;
			printf("DEBUG - on_ mouse():: have set x as %d and y as %d on index %d\n",x,y,GBL_mouseMarkCount);
			//printf("Mousey\n");
		}
	}

	int frameExtractImage(){
		// extract(load) the images into the memory.

		printf("CALL - frameExtractImage()\n");

		//local variables
		int success=2;
		//loop variables


		//-------------- code logic -----------------------------
		GBL_originalImg[0] =  cvLoadImage(GBL_basefile,1);
		if(!GBL_originalImg[0]){
			fprintf(stderr,"ERROR - frameExtractImage()::Unable to load base image\n");
			success--;
		}
		else{
			printf("DEBUG - frameExtractImage(): base Image loaded into opencv\n");
		}

		GBL_originalImg[1] =  cvLoadImage(GBL_emotionfile,1);
		if(!GBL_originalImg[1]){
			fprintf(stderr,"ERROR - frameExtractImage()::Unable to load emotion image\n");
			success--;
		}
		else{
			printf("DEBUG - frameExtractImage(): emotion Image loaded into opencv\n");
		}

		if(success == 2){
			GBL_frameCount = success;
		}


		return success-2;
	}

	int frameExtractVideo(){

		/*
		 * TODO have to implement
		 */
		return 0;
	}

	int frameExtractCamera(){

		/*
		 * TODO have to implement
		 */


		return 0;
	}


/*
 * Image process function passes each frame through series of image processing steps.
 * Current steps:
 * 			Face detect -- this is critical. without this no point in continuing. DONE
 * 			eye detect  -- DONE
 * 			nose detect -- DONE
 * 			mouth detect -- DONE
 * 			cheek detect
 * 			face segmentation
 * 			eye brow detect
 * 			lip detect
 * 	At this point live img becomes final img
 *
 */


/*
 * change log:
 *  8-4-2011 19:30
 *  		remove the display of face detected.
 *
 */
	void faceDetect(frameNum){
		// need to detect if there is a face in GBL_liveImg

		printf("CALL - faceDetect()\n");

		//local vars
		char *haarFile = "./resource/haarcascade_frontalface_alt.xml";
		IplImage *tempImg = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		IplImage *tempMask = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		CvMemStorage *storage;
		CvSeq *faces;
		CvRect *faceRect;
		CvHaarClassifierCascade *cascade_face;

		//loop vars


		//-------------- code logic -----------------------------

		//load the haar classifier file

		cascade_face = (CvHaarClassifierCascade*)cvLoad(haarFile, 0, 0, 0);
		if(!cascade_face){
			fprintf(stderr,"ERROR - faceDetect():: unable to load frontalface haar classifier\n");
			GBL_output[frameNum].isFace = 0;
			return;
		}
		storage = cvCreateMemStorage(0);

		// detect faces
		faces = cvHaarDetectObjects(GBL_liveImg,cascade_face,storage,1.1,3,0,cvSize(40,40),cvSize(50,50));
		//faces = cvHaarDetectObjects(GBL_liveImg,cascade_face,storage,1.1,3,0,cvSize(40,40));

		if (faces->total == 0){
			GBL_output[frameNum].isFace = 0;
			return;

		}

		//face is detected

		GBL_output[frameNum].isFace = 1;

		faceRect = (CvRect*)cvGetSeqElem(faces, 0);

		// now store the above rect in output
		copyRect(faceRect,&GBL_output[frameNum].face);
		/*
		 *  now that we have got the face, draw a ellipse in mask and set the mask! the commented below draws a ellips around face in live image
		 *  CODE::
		 * 		cvEllipse(GBL_liveImg, cvPoint(faceRect->x+faceRect->width/2, faceRect->y+faceRect->height/2),cvSize(faceRect->width/1.5, faceRect->height/1.5), 0, 0, 360, CV_RGB(255, 0, 0),1, 8, 0);
		 *
		 */
		cvEllipse(GBL_maskImg[frameNum], cvPoint(faceRect->x+faceRect->width/2, faceRect->y+faceRect->height/2),cvSize(faceRect->width/1.5, faceRect->height/1.5), 0, 0, 360, CV_RGB(255, 255, 255),-1, 8, 0);

		// now to set the mask
		cvCopy(GBL_liveImg,tempImg,NULL);
		cvNot(GBL_maskImg[frameNum],tempMask);
		cvOr(tempImg,tempMask,GBL_liveImg,NULL);


	// clean up of local allocated memory
		cvReleaseImage(&tempImg);
		cvReleaseImage(&tempMask);

		return;
	}


	void eyeMouthNoseDetect(int frameNum){

		// Detect eyes nose and mouth.

		printf("CALL - eyeMouthNoseDetect()\n");

		//local vars
		char *haarEye = "./resource/ojoD.xml";
		char *haarNose = "./resource/Nariz.xml";
		char *haarMouth = "./resource/Mouth.xml";
		IplImage *tempImg = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		IplImage *tempMask = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		CvMemStorage *storage;
		CvSeq *eyes,*nose,*mouth;
		CvRect *tempRect,tempStatRect;
		CvHaarClassifierCascade *cascade_eye, * cascade_nose, * cascade_mouth;
		CvFont disFont;


		//loop vars
		int loopi = 0;


		//-------------- code logic -----------------------------

		// load all haar classifier files

		cascade_eye = (CvHaarClassifierCascade*)cvLoad(haarEye, 0, 0, 0);
		if(!cascade_eye){
			fprintf(stderr,"ERROR - eyeMouthNoseDetect():: unable to load eye haar classifier\n");
			GBL_output[frameNum].isError = 1;
			return;
		}

		cascade_nose = (CvHaarClassifierCascade*)cvLoad(haarNose, 0, 0, 0);
		if(!cascade_nose){
			fprintf(stderr,"ERROR - eyeMouthNoseDetect():: unable to load nose haar classifier\n");
			GBL_output[frameNum].isError = 1;
			return;
		}


		cascade_mouth = (CvHaarClassifierCascade*)cvLoad(haarMouth, 0, 0, 0);
		if(!cascade_mouth){
			fprintf(stderr,"ERROR - eyeMouthNoseDetect():: unable to load mouth haar classifier\n");
			GBL_output[frameNum].isError = 1;
			return;
		}

		//initialize a font for display for upcoming texts

		cvInitFont(&disFont,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC,0.5,0.5,0.0,1,8);

		storage = cvCreateMemStorage(0);
		// set ROI for eye
		cvSetImageROI(GBL_liveImg, cvRect(GBL_output[frameNum].face.x, GBL_output[frameNum].face.y + (GBL_output[frameNum].face.height/5.5), GBL_output[frameNum].face.width, GBL_output[frameNum].face.height/3.0));
		// detect eyes and reset ROI
		eyes = cvHaarDetectObjects( GBL_liveImg, cascade_eye, storage,1.14, 3, 0, cvSize(25, 15), cvSize(25, 15));
		//eyes = cvHaarDetectObjects( GBL_liveImg, cascade_eye, storage,1.14, 3, 0, cvSize(25, 15));
		cvResetImageROI(GBL_liveImg);
		if(eyes->total != 2){
			// no eye is found initiate user action
			printf("DEBUG -eyeMouthNoseDetect():: no eye detected\n");
			cvCopy(GBL_liveImg,tempImg,NULL);
			cvPutText(tempImg,"Eye detect failed, mark them",cvPoint(20,20),&disFont,cvScalar(0,0,0,0));
			cvNamedWindow("Mark eyes",0);
			cvResizeWindow("Mark eyes", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("Mark eyes", tempImg);

			// now get the mouse input

			GBL_mouseMarkCount = 0;
			while(GBL_mouseMarkCount<2){
				cvSetMouseCallback("Mark eyes",on_mouse,NULL);
				cvWaitKey(0);

			}
			cvDestroyWindow("Mark eyes");

			// now we have the points. get the eye rectangles
			for(loopi=0;loopi<GBL_mouseMarkCount;loopi++){
				GBL_output[frameNum].eyes[loopi].x = GBL_mouseMark[loopi].x - 13;
				GBL_output[frameNum].eyes[loopi].y = GBL_mouseMark[loopi].y - 8;
				GBL_output[frameNum].eyes[loopi].width = 26;
				GBL_output[frameNum].eyes[loopi].height = 16;
			}

			if(GBL_output[frameNum].eyes[0].x > GBL_output[frameNum].eyes[1].x){
				tempStatRect =  GBL_output[frameNum].eyes[0];
				GBL_output[frameNum].eyes[0] = GBL_output[frameNum].eyes[1];
				GBL_output[frameNum].eyes[1] = tempStatRect;
			}


		}
		else{
			// correct eyes. store in GBL_output
			printf("DEBUG -eyeMouthNoseDetect():: two eyes correctly detected\n");

			for(loopi=0;loopi<2;loopi++){
				tempRect = (CvRect*)cvGetSeqElem( eyes, loopi );
				copyRect(tempRect,&GBL_output[frameNum].eyes[loopi]);
				//accounting for ROI
				GBL_output[frameNum].eyes[loopi].x += GBL_output[frameNum].face.x;
				GBL_output[frameNum].eyes[loopi].y += GBL_output[frameNum].face.y + (GBL_output[frameNum].face.height/5.5);
			}

			// eye[0] is left eye.. so checking

			if(GBL_output[frameNum].eyes[0].x > GBL_output[frameNum].eyes[1].x){
				tempStatRect =  GBL_output[frameNum].eyes[0];
				GBL_output[frameNum].eyes[0] = GBL_output[frameNum].eyes[1];
				GBL_output[frameNum].eyes[1] = tempStatRect;
			}

			/*
			 * testing eyes by drawing rects around eyes
			 *
			 *

			cvCopy(GBL_liveImg,tempImg,NULL);
			cvRectangle(tempImg,
							cvPoint(GBL_output[frameNum].face.x, GBL_output[frameNum].face.y),
							cvPoint(GBL_output[frameNum].face.x + GBL_output[frameNum].face.width, GBL_output[frameNum].face.y + GBL_output[frameNum].face.height),
							CV_RGB(255, 0, 0), 1, 8, 0);
			//cvSetImageROI(tempImg, cvRect(GBL_output[frameNum].face.x, GBL_output[frameNum].face.y + (GBL_output[frameNum].face.height/5.5), GBL_output[frameNum].face.width, GBL_output[frameNum].face.height/3.0));
			cvRectangle(tempImg,
					cvPoint(GBL_output[frameNum].eyes[1].x+1+GBL_output[frameNum].eyes[1].width/2.0, GBL_output[frameNum].eyes[1].y+1+ GBL_output[frameNum].eyes[1].height/2.0),
					cvPoint(GBL_output[frameNum].eyes[1].x + GBL_output[frameNum].eyes[1].width/2.0, GBL_output[frameNum].eyes[1].y + GBL_output[frameNum].eyes[1].height/2.0),
					CV_RGB(255, 0, 0), CV_FILLED, 8, 0);
			//cvResetImageROI(tempImg);
			cvNamedWindow("eye Detect test",1);
			cvShowImage("eye Detect test", tempImg);
			cvWaitKey(0);
			cvDestroyWindow("eye Detect test");

			*/
			}

		//nose draw


		cvClearMemStorage(storage);
		// done with eye processing.. now to nose processing

		// set ROI for nose
		cvSetImageROI(GBL_liveImg, cvRect(GBL_output[frameNum].eyes[0].x, GBL_output[frameNum].eyes[0].y, GBL_output[frameNum].eyes[1].x - GBL_output[frameNum].eyes[0].x +GBL_output[frameNum].eyes[1].width , GBL_output[frameNum].face.height*0.6));

		// detect nose
		nose = cvHaarDetectObjects( GBL_liveImg, cascade_nose, storage,1.14, 3, 0, cvSize(25, 15), cvSize(25, 15));
		//nose = cvHaarDetectObjects( GBL_liveImg, cascade_nose, storage,1.14, 3, 0, cvSize(25, 15));
		cvResetImageROI(GBL_liveImg);
		if(nose->total != 1){
        int x0=GBL_output[frameNum].eyes[0].x + (GBL_output[frameNum].eyes[0].width*0.6 );
        int x1=GBL_output[frameNum].eyes[1].x + (GBL_output[frameNum].eyes[0].width * 0.4);
        int y0= (GBL_output[frameNum].eyes[0].y+GBL_output[frameNum].eyes[1].y)/2 + (GBL_output[frameNum].eyes[1].x - GBL_output[frameNum].eyes[0].x)/1.70 ;
        int y1=y0+(GBL_output[frameNum].eyes[1].x - GBL_output[frameNum].eyes[0].x)*0.4;
			// now we have the points. get the eye rectangles
			GBL_output[frameNum].nose = cvRect(x0,y0,x1-x0,y1-y0);


		}
		else{
			printf("DEBUG -eyeMouthNoseDetect():: nose is detected\n");
			tempRect = (CvRect*)cvGetSeqElem( nose, 0);
			copyRect(tempRect,&GBL_output[frameNum].nose);
			//accounting for ROI
			GBL_output[frameNum].nose.x += GBL_output[frameNum].eyes[0].x;
			GBL_output[frameNum].nose.y += GBL_output[frameNum].eyes[0].y;
		}
		cvClearMemStorage(storage);

		// done with eye and nose processing.. now to mouth processing

		// set ROI for mouth

		cvSetImageROI(GBL_liveImg, cvRect(GBL_output[frameNum].eyes[0].x-10, GBL_output[frameNum].nose.y +GBL_output[frameNum].nose.height/2, GBL_output[frameNum].eyes[1].x - GBL_output[frameNum].eyes[0].x +GBL_output[frameNum].eyes[1].width +20 , GBL_output[frameNum].face.y + GBL_output[frameNum].face.height -( GBL_output[frameNum].nose.y +GBL_output[frameNum].nose.height/2) ));

		// detect mouth
		mouth = cvHaarDetectObjects( GBL_liveImg, cascade_mouth, storage,1.14, 3, 0, cvSize(25, 15), cvSize(25, 15));
		//mouth = cvHaarDetectObjects( GBL_liveImg, cascade_mouth, storage,1.14, 3, 0, cvSize(25, 15));
		cvResetImageROI(GBL_liveImg);
		if(mouth->total != 1){
			//no mouth detected
			printf("DEBUG -eyeMouthNoseDetect():: no mouth detected\n");
			cvCopy(GBL_liveImg,tempImg,NULL);
			cvPutText(tempImg,"Mouth detect failed, mark them",cvPoint(20,20),&disFont,cvScalar(0,0,0,0));
			cvNamedWindow("Mark mouth",0);
			cvResizeWindow("Mark mouth", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("Mark mouth", tempImg);

			// now get the mouse input

			GBL_mouseMarkCount = 0;
			while(GBL_mouseMarkCount<4){
				cvSetMouseCallback("Mark mouth",on_mouse,NULL);
				cvWaitKey(0);

			}
			cvDestroyWindow("Mark mouth");

			// now we have the points. get the mouth rectangles
			GBL_output[frameNum].mouth = cvRect(GBL_mouseMark[0].x,GBL_mouseMark[2].y,GBL_mouseMark[1].x - GBL_mouseMark[0].x, GBL_mouseMark[3].y - GBL_mouseMark[2].y);


		}
		else{
			printf("DEBUG -eyeMouthNoseDetect():: mouth is detected\n");
			tempRect = (CvRect*)cvGetSeqElem( mouth, 0);
			copyRect(tempRect,&GBL_output[frameNum].mouth);
			//accounting for ROI
			GBL_output[frameNum].mouth.x += GBL_output[frameNum].eyes[0].x - 10;
			GBL_output[frameNum].mouth.y += GBL_output[frameNum].nose.y +GBL_output[frameNum].nose.height/2;
		}








		//display eyes and nose and mouth if verbose
		if(GBL_verbose){

			cvCopy(GBL_liveImg,tempImg,NULL);
			for(loopi=0;loopi<2;loopi++){
			cvRectangle(tempImg,
					cvPoint(GBL_output[frameNum].eyes[loopi].x, GBL_output[frameNum].eyes[loopi].y),
					cvPoint(GBL_output[frameNum].eyes[loopi].x + GBL_output[frameNum].eyes[loopi].width, GBL_output[frameNum].eyes[loopi].y + GBL_output[frameNum].eyes[loopi].height),
					CV_RGB(255, 0, 0), 1, 8, 0);
			}

			cvNamedWindow("eye Detect test",0);
			cvResizeWindow("eye Detect test", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("eye Detect test", tempImg);
			cvWaitKey(0);
			cvDestroyWindow("eye Detect test");

			cvRectangle(tempImg,
					cvPoint(GBL_output[frameNum].nose.x, GBL_output[frameNum].nose.y),
					cvPoint(GBL_output[frameNum].nose.x + GBL_output[frameNum].nose.width, GBL_output[frameNum].nose.y + GBL_output[frameNum].nose.height),
					CV_RGB(255, 0, 0), 1, 8, 0);
			cvNamedWindow("nose Detect test",0);
			cvResizeWindow("nose Detect test", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("nose Detect test", tempImg);
			cvWaitKey(0);
			cvDestroyWindow("nose Detect test");

			cvRectangle(tempImg,
					cvPoint(GBL_output[frameNum].mouth.x, GBL_output[frameNum].mouth.y),
					cvPoint(GBL_output[frameNum].mouth.x + GBL_output[frameNum].mouth.width, GBL_output[frameNum].mouth.y + GBL_output[frameNum].mouth.height),
					CV_RGB(0, 255, 0), 1, 8, 0);
			cvNamedWindow("mouth Detect test",0);
			cvResizeWindow("mouth Detect test", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("mouth Detect test", tempImg);
			cvWaitKey(0);
			cvDestroyWindow("mouth Detect test");



		}

		// clean up of local allocated memory
		cvReleaseImage(&tempImg);
		cvReleaseImage(&tempMask);

		return;
	}

	int faceSegment(int frameNum,int step5,int isIll,int isMirror){

		// cut the outilne of face
		// have mirroring and do away with illuminance regularization

		printf("CALL - faceSegment()\n");

		//local variables
		IplImage *ycrcb,*res1,*res2,*out1,*out2;
		IplImage *reduced,*tempreduced; // A [M/4 x N/4] matrix to store transformation info
		uchar *data,cr,cb,bitdata;		// to access the data in images
		double stddata[4][4];
 		double stdDev;
		int step,dxy;
		CvScalar stdDevS;

		//loop vars
		int loopi,loopj,loop1i,loop1j,loopk;


		//------------------------- code logic ---------------------------------

		// Init all local images
		ycrcb = cvCreateImage(cvGetSize(GBL_originalImg[frameNum]),IPL_DEPTH_8U,3);
		res1 = cvCreateImage(cvGetSize(GBL_originalImg[frameNum]),IPL_DEPTH_8U,1);
		out1 = cvCreateImage(cvGetSize(GBL_originalImg[frameNum]),IPL_DEPTH_8U,1);
		res2 = cvCreateImage(cvGetSize(GBL_originalImg[frameNum]),IPL_DEPTH_8U,1);
		out2 = cvCreateImage(cvGetSize(GBL_originalImg[frameNum]),IPL_DEPTH_8U,1);
		reduced = cvCreateImage(cvSize((GBL_originalImg[frameNum]->width)/4,(GBL_originalImg[frameNum]->height)/4),IPL_DEPTH_8U,1);
		tempreduced =cvCreateImage(cvSize((GBL_originalImg[frameNum]->width)/4,(GBL_originalImg[frameNum]->height)/4),IPL_DEPTH_8U,1);
		cvZero(res1);
		cvZero(res2);
		cvZero(reduced);
		cvZero(tempreduced);

		// algo step 1
		cvCvtColor(GBL_originalImg[frameNum],ycrcb,CV_BGR2YCrCb);

		// conversion to ycrcb


		data = (uchar *)ycrcb->imageData;
		step = ycrcb->widthStep/sizeof(uchar);

		for(loopi=0;loopi<ycrcb->height;loopi++){
			for(loopj=0;loopj<ycrcb->width;loopj++){
				cr = data[loopi*step+loopj*3+1];
				cb = data[loopi*step+loopj*3+2];
				if(cr>133 && cr<173 && cb>77 && cb<127){
					((uchar *)(res1->imageData + loopi*res1->widthStep))[loopj]=255;
				}
			}
		}
        cvCopyImage(res1,out1);

		// algo step 2a
		data = (uchar *)res1->imageData;
		step = res1->widthStep/sizeof(uchar);

		for(loopi=2;loopi<res1->height;loopi+=4){
			for(loopj=2;loopj<res1->width;loopj+=4){

				//calculate dxy
				dxy=0;
				for(loop1i=loopi-2;loop1i<loopi+2;loop1i++){
					for(loop1j=loopj-2;loop1j<loopj+2;loop1j++){
						bitdata = data[loop1i*step+loop1j];
						if(bitdata == 255){
							dxy++;
						}
					}
				}

				// store dxy now. defer calculating output reps only if on verbose
				if(dxy==16){
					((uchar *)(reduced->imageData + (loop1i-2)/4*reduced->widthStep))[(loop1j-2)/4]=255;
				}
				else if(dxy>0){
					((uchar *)(reduced->imageData + (loop1i-2)/4*reduced->widthStep))[(loop1j-2)/4]=111;
				}


				/*
				//set res2 based on dxy
				for(loop1i=loopi-2;loop1i<loopi+2;loop1i++){
					for(loop1j=loopj-2;loop1j<loopj+2;loop1j++){
						if(dxy==16){
							((uchar *)(res2->imageData + loop1i*res2->widthStep))[loop1j]=255;
						}
						else if(dxy>0){
							((uchar *)(res2->imageData + loop1i*res2->widthStep))[loop1j]=111;
						}
					}
				}
				*/

			}
		}

/*
		//populate res2 with data in reduced
		data = (uchar *)reduced->imageData;
		step = reduced->widthStep/sizeof(uchar);

		for(loopi=0;loopi<reduced->height;loopi++){
			for(loopj=0;loopj<reduced->width;loopj++){

				for(loop1i=0;loop1i<4;loop1i++){
					for(loop1j=0;loop1j<4;loop1j++){

						((uchar *)(res2->imageData + (loopi*4+loop1i)*res2->widthStep))[loopj*4+loop1j]=data[loopi*step+loopj];
					}
				}

			}
		}
		cvNamedWindow("res2",CV_WINDOW_AUTOSIZE);
		cvShowImage("res2", res2);
		cvWaitKey(0);
		cvDestroyWindow("res2");
*/

		// algo step 2b
		// set border values to 0
		for(loopi=0;loopi<reduced->height;loopi++){
			((uchar *)(reduced->imageData + loopi*reduced->widthStep))[0]=000;
			((uchar *)(reduced->imageData + loopi*reduced->widthStep))[reduced->width - 1]=000;
		}

		for(loopj=0;loopj<reduced->width;loopj++){
			((uchar *)(reduced->imageData + 0*reduced->widthStep))[loopj]=000;
			((uchar *)(reduced->imageData + (reduced->height -1)*reduced->widthStep))[loopj]=000;
		}

		// eroding full density points if surrounded by less than 5 full density points

		tempreduced = cvCloneImage(reduced);
		data = (uchar *)reduced->imageData;
		step = reduced->widthStep/sizeof(uchar);

		for(loopi=1;loopi<reduced->height-1;loopi++){
			for(loopj=1;loopj<reduced->width-1;loopj++){

				dxy=0;
				bitdata = data[loopi*step+loopj];
				if(bitdata == 255){
					// this is full density point, check if 3x3 around has more than 5 full density points
					for(loop1i=loopi-2;loop1i<loopi+2;loop1i++){
						for(loop1j=loopj-2;loop1j<loopj+2;loop1j++){
							bitdata = data[loop1i*step+loop1j];
							if(bitdata == 255){
								dxy++;
							}
						}
					}
					if(dxy<5){
						((uchar *)(tempreduced->imageData + loopi*tempreduced->widthStep))[loopj]=000;
					}
				}
			}
		}

		// dialate points

		reduced = cvCloneImage(tempreduced);
		data = (uchar *)tempreduced->imageData;
		step = tempreduced->widthStep/sizeof(uchar);

		for(loopi=1;loopi<tempreduced->height-1;loopi++){
			for(loopj=1;loopj<tempreduced->width-1;loopj++){

				dxy=0;
				bitdata = data[loopi*step+loopj];
				if(bitdata != 255){
					// this is not full density point, check if 3x3 around has more than 2 full density points
					for(loop1i=loopi-2;loop1i<loopi+2;loop1i++){
						for(loop1j=loopj-2;loop1j<loopj+2;loop1j++){
							bitdata = data[loop1i*step+loop1j];
							if(bitdata == 255){
								dxy++;
							}
						}
					}
					if(dxy>2){
						((uchar *)(reduced->imageData + loopi*reduced->widthStep))[loopj]=255;
					}
				}

			}

		}

		// produce density map
		data = (uchar *)reduced->imageData;
		step = reduced->widthStep/sizeof(uchar);

		for(loopi=0;loopi<tempreduced->height;loopi++){
			for(loopj=0;loopj<tempreduced->width;loopj++){
				bitdata = data[loopi*step+loopj];
				if(bitdata == 111){
					data[loopi*step+loopj] = 255;
				}
			}
		}





		//populate res2 with data in reduced
		data = (uchar *)reduced->imageData;
		step = reduced->widthStep/sizeof(uchar);

		for(loopi=0;loopi<reduced->height;loopi++){
			for(loopj=0;loopj<reduced->width;loopj++){

				for(loop1i=0;loop1i<4;loop1i++){
					for(loop1j=0;loop1j<4;loop1j++){

						((uchar *)(res2->imageData + (loopi*4+loop1i)*res2->widthStep))[loopj*4+loop1j]=data[loopi*step+loopj];
					}
				}

			}
		}
       cvCopyImage(res2,out2);


		// implementing 3rd step only if asked for -- it is skipped by default [NOT RECOMENDED]
		if(isIll){

			printf("DEBUG - faceSegment():: called for illumination regularization code\n");

			tempreduced = cvCloneImage(reduced);
			data = (uchar *)reduced->imageData;
			step = reduced->widthStep/sizeof(uchar);

			for(loopi=0;loopi<reduced->height;loopi++){
				for(loopj=0;loopj<reduced->width;loopj++){

					for(loop1i=0;loop1i<4;loop1i++){
						for(loop1j=0;loop1j<4;loop1j++){

							//((uchar *)(res2->imageData + (loopi*4+loop1i)*res2->widthStep))[loopj*4+loop1j]=data[loopi*step+loopj];
							stddata[loop1i][loop1j] = (int)((uchar *)(ycrcb->imageData + (loopi*4+loop1i)*ycrcb->widthStep))[loopj*4+loop1j*ycrcb->nChannels + 0];

						}
					}
					CvMat stdMat  = cvMat(4, 4, CV_64FC1, stddata);
					cvAvgSdv((CvArr *)&stdMat,NULL,&stdDevS,NULL);
					stdDev = stdDevS.val[0];
					if(stdDev<2.00){
						((uchar *)(reduced->imageData + (loopi)*reduced->widthStep))[loopj] = 000;
					}
					//printf("%G\n",stdDev);
				}
			}
		}

		// Now to 4a
		tempreduced = cvCloneImage(reduced);
		data = (uchar *)reduced->imageData;
		step = reduced->widthStep/sizeof(uchar);

		for(loopi=1;loopi<reduced->height-1;loopi++){
			for(loopj=1;loopj<reduced->width-1;loopj++){

				dxy=0;
				bitdata = data[loopi*step+loopj];
				if(bitdata == 255){
					// this is detected pixel, check if 3x3 around has more than 3 detected pixels
					for(loop1i=loopi-2;loop1i<loopi+2;loop1i++){
						for(loop1j=loopj-2;loop1j<loopj+2;loop1j++){
							bitdata = data[loop1i*step+loop1j];
							if(bitdata == 255){
								dxy++;
							}
						}
					}
					if(dxy<3){
						// less than 3 detected pixels. so make it 000
						((uchar *)(tempreduced->imageData + loopi*tempreduced->widthStep))[loopj]=000;
					}
				}
			}
		}
		// now if a 000 point has more than 5 ones around make it one
		reduced = cvCloneImage(tempreduced);
		data = (uchar *)tempreduced->imageData;
		step = tempreduced->widthStep/sizeof(uchar);

		for(loopi=1;loopi<tempreduced->height-1;loopi++){
			for(loopj=1;loopj<tempreduced->width-1;loopj++){

				dxy=0;
				bitdata = data[loopi*step+loopj];
				if(bitdata != 255){
					// this is 000. see if more than 5 ones in 3x3 exists
					for(loop1i=loopi-2;loop1i<loopi+2;loop1i++){
						for(loop1j=loopj-2;loop1j<loopj+2;loop1j++){
							bitdata = data[loop1i*step+loop1j];
							if(bitdata == 255){
								dxy++;
							}
						}
					}
					if(dxy>5){
						((uchar *)(reduced->imageData + loopi*reduced->widthStep))[loopj]=255;
					}
				}

			}

		}
/*
		data = (uchar *)reduced->imageData;
		step = reduced->widthStep/sizeof(uchar);

		for(loopi=0;loopi<reduced->height;loopi++){
			for(loopj=0;loopj<reduced->width;loopj++){

				for(loop1i=0;loop1i<4;loop1i++){
					for(loop1j=0;loop1j<4;loop1j++){

						((uchar *)(res2->imageData + (loopi*4+loop1i)*res2->widthStep))[loopj*4+loop1j]=data[loopi*step+loopj];
					}
				}

			}
		}
		cvNamedWindow("res2",CV_WINDOW_AUTOSIZE);
		cvShowImage("res2", res2);
		cvWaitKey(0);
		cvDestroyWindow("res2");
*/
		// step 5
		// horizontal scanning
		// here using 4 as connection std.. while integrating take into consideratin the CIF std which is 352 × 288,
		// but using 1 as default.. it is working well
		// any group of less than 4 horizontally connected pixels with value of BLACK will be made white
		// the data is in reduced, calc will be stored in tempreduced
		// run the following algo step5 num of times.
	for(loopk=0;loopk<step5;loopk++){
			tempreduced = cvCloneImage(reduced);
			data = (uchar *)reduced->imageData;
			step = reduced->widthStep/sizeof(uchar);

			for(loopi=0;loopi<reduced->height;loopi++){
				for(loopj=2;loopj<reduced->width-2;loopj++){
					//check the surrounding 4 pixels
					dxy=0;
					for(loop1j = loopj-2;loop1j<loopj+2;loop1j++){
						bitdata = data[loopi*step+loop1j];
						if(bitdata == 255){
							dxy=1;
						}

					}
					if(dxy==1){
						//some surrounding pixel was white so make this white
						((uchar *)(tempreduced->imageData + (loopi)*tempreduced->widthStep))[loop1j]=255;
					}

				}
			}

			// now to vertical scanning

			reduced = cvCloneImage(tempreduced);
			data = (uchar *)tempreduced->imageData;
			step = tempreduced->widthStep/sizeof(uchar);

			for(loopi=2;loopi<tempreduced->height-2;loopi++){
				for(loopj=0;loopj<reduced->width;loopj++){
					//check the surrounding 4 pixels
					dxy=0;
					for(loop1i = loopi-2;loop1i<loopi+2;loop1i++){
						bitdata = data[loop1i*step+loopj];
						if(bitdata == 255){
							dxy=1;
						}

					}
					if(dxy==1){
						//some surrounding pixel was white so make this white
						((uchar *)(tempreduced->imageData + (loop1i)*tempreduced->widthStep))[loopj]=255;
					}

				}
			}
	}

		data = (uchar *)reduced->imageData;
		step = reduced->widthStep/sizeof(uchar);

		for(loopi=0;loopi<reduced->height;loopi++){
			for(loopj=0;loopj<reduced->width;loopj++){

				for(loop1i=0;loop1i<4;loop1i++){
					for(loop1j=0;loop1j<4;loop1j++){

						((uchar *)(res2->imageData + (loopi*4+loop1i)*res2->widthStep))[loopj*4+loop1j]=data[loopi*step+loopj];
					}
				}

			}
		}
		/*
		cvNamedWindow("res2",CV_WINDOW_AUTOSIZE);
		cvShowImage("res2", res2);
		cvWaitKey(0);
		cvDestroyWindow("res2");
		 */
		// now to merge the reduced thing and res1 to produce the contour
		for(loopi=0;loopi<res2->height;loopi++){
			for(loopj=0;loopj<res2->width;loopj++){
				if ((((uchar *)(res2->imageData + loopi*res2->widthStep))[loopj])==255){
				((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 0]= 000;
				((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 1]= 000;
				((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 2]= 000;
				}
				else{
					((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 0]= 255;
					((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 1]= 255;
					((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 2]= 255;


				}
			}
		}

		// if asked to mirror. then mirror mask along the center of two eyes

		if(isMirror){
			printf("is mirroring\n");
			int centerx = (GBL_output[frameNum].eyes[0].x +GBL_output[frameNum].eyes[1].x +GBL_output[frameNum].eyes[0].width)/2;
			centerx +=2;
			int copyWidth = 0;
			if(centerx>GBL_output[frameNum].imgWidth/2 ){
				copyWidth = GBL_output[frameNum].imgWidth - centerx -5;
			}
			else{
				copyWidth = centerx -5;
			}
			//set all pixels to right of centerx+1 to the mirror on other side
			for(loopi=0;loopi<GBL_maskImg[frameNum]->height;loopi++){
				for(loopj=centerx;loopj<centerx +  copyWidth;loopj++){
					((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 0]=((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[(2*centerx - loopj)*GBL_maskImg[frameNum]->nChannels + 0];
					((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 1]=((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[(2*centerx - loopj)*GBL_maskImg[frameNum]->nChannels + 1];
					((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 2]=((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[(2*centerx - loopj)*GBL_maskImg[frameNum]->nChannels + 2];
				}
			}

		}

		cvOr(GBL_originalImg[frameNum],GBL_maskImg[frameNum],GBL_liveImg,NULL);

		// handle all displaying and cleaning up
		if(GBL_verbose){
		    /*
		    // not required always
            cvNamedWindow("ycrcb",0);
            cvResizeWindow("ycrcb", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("ycrcb", ycrcb);
            cvWaitKey(0);
            cvDestroyWindow("ycrcb");
            */


            cvNamedWindow("segmented_face",0);
            cvResizeWindow("segmented_face", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("segmented_face", GBL_liveImg);
            cvWaitKey(0);
            cvDestroyWindow("segmented_face");


           cvNamedWindow("Generated Mask",0);
            cvResizeWindow("Generated Mask", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("Generated Mask", GBL_maskImg[frameNum]);
            cvWaitKey(0);
            cvDestroyWindow("Generated Mask");


            cvNamedWindow("ycrcb",0);
            cvResizeWindow("ycrcb", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("ycrcb", ycrcb);
            cvWaitKey(0);
            cvDestroyWindow("ycrcb");

            cvNamedWindow("inital threshold mask",0);
            cvResizeWindow("inital threshold mask", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("inital threshold mask", out1);
            cvWaitKey(0);
            cvDestroyWindow("inital threshold mask");

            cvNamedWindow("Algos final result",0);
            cvResizeWindow("Algos final result", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("Algos final result", out2);
            cvWaitKey(0);
            cvDestroyWindow("Algos final result");

            cvNamedWindow("Generated Mask",0);
            cvResizeWindow("Generated Mask", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("Generated Mask", GBL_maskImg[frameNum]);
            cvWaitKey(0);
            cvDestroyWindow("Generated Mask");



            cvReleaseImage(&ycrcb);
            cvReleaseImage(&res1);
            cvReleaseImage(&res2);
		}
		return 0;
	}


	void connectedRecursive(IplImage* inImg,int i,int j,int frameNum){

		// a recursive function, which given a point in cluster which is white, makes the whole cluster black
		// used for advanced hair removing



		int loopi,loopj;
		loopi = i;
		loopj=j;
		((uchar *)(inImg->imageData + i*inImg->widthStep))[j]=000;
		((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 0]= 255;
		((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 1]= 255;
		((uchar *)(GBL_maskImg[frameNum]->imageData + loopi*GBL_maskImg[frameNum]->widthStep))[loopj*GBL_maskImg[frameNum]->nChannels + 2]= 255;

		//check left, j has width, i has height
		if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j+1])==255){
			connectedRecursive(inImg,i,j+1,frameNum);
		}
		// check right
		if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j-1])==255){
			connectedRecursive(inImg,i,j-1,frameNum);
		}
		//check down
		if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j])==255){
			connectedRecursive(inImg,i+1,j,frameNum);
		}
		//check up
		//check down
		if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j])==255){
			connectedRecursive(inImg,i-1,j,frameNum);
		}

		return;
	}

	void connectedRecursiveCount(IplImage* inImg,int i,int j,int x0,int y0,int x1,int y1){

		// a recursive function count to see the area of connected component

		int loopi,loopj;
		loopi = i;
		loopj=j;
		// if pixel is out of range return
		if(j<x0 || j>x1 || i<y0 || i>y1){
			return;
		}

        GBL_eyebrow[0]++;
        GBL_eyebrow[1]+=j;
        GBL_eyebrow[2]+=i;
		((uchar *)(inImg->imageData + i*inImg->widthStep))[j]=000;
		//check left, j has width, i has height
		if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j+1])==255){
			connectedRecursiveCount(inImg,i,j+1,x0,y0,x1,y1);
		}
		// check right
		if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j-1])==255){
			connectedRecursiveCount(inImg,i,j-1,x0,y0,x1,y1);
		}
		//check down
		if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j])==255){
			connectedRecursiveCount(inImg,i+1,j,x0,y0,x1,y1);
		}
		//check up
		//check down
		if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j])==255){
			connectedRecursiveCount(inImg,i-1,j,x0,y0,x1,y1);
		}

		return;
	}

    void connectedRecursiveCountLip(IplImage* inImg,int i,int j){

        		// a recursive function count to see the area of connected component

		int loopi,loopj;
		loopi = i;
		loopj=j;

        GBL_eyebrow[0]++;
        GBL_eyebrow[1]+=j;
        GBL_eyebrow[2]+=i;
 		((uchar *)(inImg->imageData + i*inImg->widthStep))[j]=000;
		//check left, j has width, i has height
		if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j+1])==255){
			connectedRecursiveCountLip(inImg,i,j+1);
		}
		else if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j+2])==255){
			connectedRecursiveCountLip(inImg,i,j+1);
		}
		// check right
		if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j-1])==255){
			connectedRecursiveCountLip(inImg,i,j-1);
		}
		else if((((uchar *)(inImg->imageData + i*inImg->widthStep))[j-1])==255){
			connectedRecursiveCountLip(inImg,i,j-1);
		}
		//check down
		if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j])==255){
			connectedRecursiveCountLip(inImg,i+1,j);
		}
		else if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j])==255){
			connectedRecursiveCountLip(inImg,i+1,j);
		}
		//check up
		//check down
		if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j])==255){
			connectedRecursiveCountLip(inImg,i-1,j);
		}
		else if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j])==255){
			connectedRecursiveCountLip(inImg,i-1,j);
		}


// the other 4 traversals
		//check left, j has width, i has height
		if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j+1])==255){
			connectedRecursiveCountLip(inImg,i+1,j+1);
		}
		else if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j+2])==255){
			connectedRecursiveCountLip(inImg,i+1,j+1);
		}
		// check right
		if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j-1])==255){
			connectedRecursiveCountLip(inImg,i+1,j-1);
		}
		else if((((uchar *)(inImg->imageData + (i+1)*inImg->widthStep))[j-1])==255){
			connectedRecursiveCountLip(inImg,i+1,j-1);
		}
		//check down
		if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j+1])==255){
			connectedRecursiveCountLip(inImg,i-1,j+1);
		}
		else if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j+1])==255){
			connectedRecursiveCountLip(inImg,i-1,j+1);
		}
		//check up
		//check down
		if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j-1])==255){
			connectedRecursiveCountLip(inImg,i-1,j-1);
		}
		else if((((uchar *)(inImg->imageData + (i-1)*inImg->widthStep))[j-1])==255){
			connectedRecursiveCountLip(inImg,i-1,j-1);
		}



		return;

    }
	void fineCleanHair(int frameNum){

		// Actually meant for eyebrow.. BEWARE of wrong name
		// fine cleans hair, then sets threshold to get the eyebrows..


		printf  ("CALL - fineCleanHair()\n");

		//local vars

		IplImage *tempImg,*imgThreshed,*livecopy ;
		int topy,centerx;
		int eyebrow[10][3];
		CvFont disFont;

		//loop vars
		int loopi,loopj ;

		//------------------------ code loogic ---------------------------------

		cvInitFont(&disFont,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC,0.5,0.5,0.0,1,8);

		for(loopi=0;loopi<10;loopi++){
			for(loopj=0;loopj<3;loopj++){
				eyebrow[loopi][loopj] = 0;
			}
		}
		tempImg = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		livecopy = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		imgThreshed = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), 8, 1);


		// set the threshold, then normalize the imgThreshed
		cvCopyImage(GBL_liveImg,tempImg);
		cvInRangeS(tempImg, cvScalar(0, 0, 0,0), cvScalar(50, 50, 50,0), imgThreshed);

		// now to remove parts that are not probably eyebrows. get a hair point above the ROI of eyebrow. Then remove all connected components
		// of this

		topy = (GBL_output[frameNum].eyes[0].y +GBL_output[frameNum].eyes[1].y )/2;
		topy = GBL_output[frameNum].face.y + (topy - GBL_output[frameNum].face.y)*0.50;

		for(loopi=0;loopi<topy;loopi++){
			for(loopj=GBL_output[frameNum].face.x;loopj<GBL_output[frameNum].face.x+GBL_output[frameNum].face.width;loopj++){
				if((((uchar *)(imgThreshed->imageData + loopi*imgThreshed->widthStep))[loopj])==255){
					// this point is white.. so call the recursive fun
					printf("CALLING - connectedRecursive()\n");
					connectedRecursive(imgThreshed,loopi,loopj,frameNum);

				}
			}
		}

		/*
		 * draw for test
		 *
		 cvCopyImage(GBL_liveImg,tempImg);
		 cvRectangle(tempImg,
						cvPoint(GBL_output[frameNum].face.x,0),
						cvPoint(GBL_output[frameNum].face.x+GBL_output[frameNum].face.width, topy),
						CV_RGB(0, 0, 255), 2, 8, 0);
        			cvNamedWindow("test roi",0);
			cvResizeWindow("test roi", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("test roi", tempImg);
			cvWaitKey(0);
			cvDestroyWindow("test roi");
*/


		// now imgThreshed has the values of eyebrow region. To detect it.
		// set the ROI
		centerx = (GBL_output[frameNum].eyes[0].x +GBL_output[frameNum].eyes[1].x +GBL_output[frameNum].eyes[0].width)/2;
		/* ROI is demonstrated in the below rect
		cvRectangle(tempImg,
						cvPoint(centerx,topy),
						cvPoint(GBL_output[frameNum].eyes[1].x+GBL_output[frameNum].eyes[1].width+10, GBL_output[frameNum].eyes[1].y +(GBL_output[frameNum].eyes[1].height)/2),
						CV_RGB(0, 255, 0), 2, 8, 0);
	*/

		//left eye brow 1
		tempImg = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), 8, 1); //need this to be a copy of imgThres
		cvCopyImage(imgThreshed,tempImg);
		int count=0;
		int x0 = GBL_output[frameNum].eyes[0].x-10;
		int x1 = centerx/2 + (x0+10)/2;
		int y0 = topy;
		int y1  = (GBL_output[frameNum].eyes[1].y+GBL_output[frameNum].eyes[0].y +GBL_output[frameNum].eyes[1].height)/2;
		for(loopi=y0;loopi<y1;loopi++){
			for(loopj =x0;loopj<x1;loopj++ ){
				if((((uchar *)(tempImg->imageData + loopi*tempImg->widthStep))[loopj])==255){
				// get the largest connected component in this area
					GBL_eyebrow[0]=0; //count
					GBL_eyebrow[1]=0; // x-coor width
					GBL_eyebrow[2]=0; //y-coor height
				printf("CALLING - connectedRecursiveCount()\n");
				connectedRecursiveCount(tempImg,loopi,loopj,x0,y0,x1,y1);
				// printf("COUNT::%d",GBL_eyebrow[0]);
				eyebrow[count][0] = GBL_eyebrow[0];
				eyebrow[count][1] = GBL_eyebrow[1]/GBL_eyebrow[0]; // width
				eyebrow[count][2] = GBL_eyebrow[2]/GBL_eyebrow[0]; // heigth
				count++;
				}

			}
		}
		// determine the largest count and store its avg points
		int max_index = 0;
		for(loopi=1;loopi<count;loopi++){
			if(eyebrow[loopi][0]>eyebrow[max_index][0]){
				max_index = loopi;
			}
			else if(eyebrow[loopi][0] == eyebrow[max_index][0]){
				if(eyebrow[loopi][2]>eyebrow[max_index][2]){
					max_index = loopi;
				}

			}
		}
		if(eyebrow[max_index][1]<x0 || eyebrow[max_index][1]>x1 || eyebrow[max_index][1]<y0 || eyebrow[max_index][1]>y1){
			GBL_output[frameNum].lbrow[0] = cvPoint(eyebrow[max_index][1],eyebrow[max_index][2]);
		}
		else{
			GBL_output[frameNum].lbrow[0] = cvPoint(0,0);
		}


		//left brow 2
		count=0;
		x0 = x1;
		x1 = centerx;
		y0 = topy;
		y1  = (GBL_output[frameNum].eyes[1].y+GBL_output[frameNum].eyes[0].y +GBL_output[frameNum].eyes[1].height)/2;
		for(loopi=y0;loopi<y1;loopi++){
			for(loopj =x0;loopj<x1;loopj++ ){
				if((((uchar *)(tempImg->imageData + loopi*tempImg->widthStep))[loopj])==255){
				// get the largest connected component in this area
					GBL_eyebrow[0]=0; //count
					GBL_eyebrow[1]=0; // x-coor width
					GBL_eyebrow[2]=0; //y-coor height
				printf("CALLING - connectedRecursiveCount()\n");
				connectedRecursiveCount(tempImg,loopi,loopj,x0,y0,x1,y1);
				// printf("COUNT::%d",GBL_eyebrow[0]);
				eyebrow[count][0] = GBL_eyebrow[0];
				eyebrow[count][1] = GBL_eyebrow[1]/GBL_eyebrow[0]; // width
				eyebrow[count][2] = GBL_eyebrow[2]/GBL_eyebrow[0]; // heigth
				count++;
				}

			}
		}
		// determine the largest count and store its avg points
		max_index = 0;
		for(loopi=1;loopi<count;loopi++){
			if(eyebrow[loopi][0]>eyebrow[max_index][0]){
				max_index = loopi;
			}
			else if(eyebrow[loopi][0] == eyebrow[max_index][0]){
				if(eyebrow[loopi][2]>eyebrow[max_index][2]){
					max_index = loopi;
				}

			}
		}
		// check if value is acceptable
		if(eyebrow[max_index][1]<x0 || eyebrow[max_index][1]>x1 || eyebrow[max_index][1]<y0 || eyebrow[max_index][1]>y1){
			GBL_output[frameNum].lbrow[1] = cvPoint(eyebrow[max_index][1],eyebrow[max_index][2]);
		}
		else{
			GBL_output[frameNum].lbrow[1] = cvPoint(0,0);
		}

		// In case both above left failed.. init user input
		if(GBL_output[frameNum].lbrow[0].x == 0 && GBL_output[frameNum].lbrow[0].y ==0
				&& GBL_output[frameNum].lbrow[1].x == 0 && GBL_output[frameNum].lbrow[1].y == 0){

			printf("DEBUG -fineCleanHair():: no left brow detected\n");
			cvCopy(GBL_liveImg,livecopy,NULL);
			cvPutText(livecopy,"left eye brow detect failed, mark them",cvPoint(20,20),&disFont,cvScalar(0,0,0,0));
			cvNamedWindow("Mark left eye brow",0);
			cvResizeWindow("Mark left eye brow", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("Mark left eye brow", livecopy);

			// now get the mouse input

			GBL_mouseMarkCount = 0;
			while(GBL_mouseMarkCount<2){
				cvSetMouseCallback("Mark left eye brow",on_mouse,NULL);
				cvWaitKey(0);

			}
			cvDestroyWindow("Mark left eye brow");

			// now we have co-ordinates, put them in
			for(loopi=0;loopi<2;loopi++){
				GBL_output[frameNum].lbrow[loopi].x = GBL_mouseMark[loopi].x;
				GBL_output[frameNum].lbrow[loopi].y = GBL_mouseMark[loopi].y;
			}
		}
		// in case only one of the eyebrows failed
		else if(GBL_output[frameNum].lbrow[0].x == 0 && GBL_output[frameNum].lbrow[0].y ==0){
			GBL_output[frameNum].lbrow[0] = GBL_output[frameNum].lbrow[1];

		}
		else if(GBL_output[frameNum].lbrow[1].x == 0 && GBL_output[frameNum].lbrow[1].y ==0){
					GBL_output[frameNum].lbrow[1] = GBL_output[frameNum].lbrow[0];

		}

		// right eye brow 1

		count=0;
		x0 = centerx;
		x1 = centerx + (GBL_output[frameNum].eyes[1].width)/2;
		y0 = topy;
		y1  = (GBL_output[frameNum].eyes[1].y+GBL_output[frameNum].eyes[0].y +GBL_output[frameNum].eyes[1].height)/2;
		for(loopi=y0;loopi<y1;loopi++){
			for(loopj =x0;loopj<x1;loopj++ ){
				if((((uchar *)(tempImg->imageData + loopi*tempImg->widthStep))[loopj])==255){
				// get the largest connected component in this area
					GBL_eyebrow[0]=0; //count
					GBL_eyebrow[1]=0; // x-coor width
					GBL_eyebrow[2]=0; //y-coor height
				printf("CALLING - connectedRecursiveCount()\n");
				connectedRecursiveCount(tempImg,loopi,loopj,x0,y0,x1,y1);
				// printf("COUNT::%d",GBL_eyebrow[0]);
				eyebrow[count][0] = GBL_eyebrow[0];
				eyebrow[count][1] = GBL_eyebrow[1]/GBL_eyebrow[0]; // width
				eyebrow[count][2] = GBL_eyebrow[2]/GBL_eyebrow[0]; // heigth
				count++;
				}

			}
		}
		// determine the largest count and store its avg points
		max_index = 0;
		for(loopi=1;loopi<count;loopi++){
			if(eyebrow[loopi][0]>eyebrow[max_index][0]){
				max_index = loopi;
			}
			else if(eyebrow[loopi][0] == eyebrow[max_index][0]){
				if(eyebrow[loopi][2]>eyebrow[max_index][2]){
					max_index = loopi;
				}

			}
		}
		// check if value is acceptable
		if(eyebrow[max_index][1]<x0 || eyebrow[max_index][1]>x1 || eyebrow[max_index][1]<y0 || eyebrow[max_index][1]>y1){
			GBL_output[frameNum].rbrow[0] = cvPoint(eyebrow[max_index][1],eyebrow[max_index][2]);
		}
		else{
			GBL_output[frameNum].rbrow[0] = cvPoint(0,0);
		}

		// right eye brow 2

		count=0;
		x0 = x1;
		x1 = centerx + GBL_output[frameNum].eyes[1].width + 10;
		y0 = topy;
		y1  = (GBL_output[frameNum].eyes[1].y+GBL_output[frameNum].eyes[0].y +GBL_output[frameNum].eyes[1].height)/2;
		for(loopi=y0;loopi<y1;loopi++){
			for(loopj =x0;loopj<x1;loopj++ ){
				if((((uchar *)(tempImg->imageData + loopi*tempImg->widthStep))[loopj])==255){
				// get the largest connected component in this area
					GBL_eyebrow[0]=0; //count
					GBL_eyebrow[1]=0; // x-coor width
					GBL_eyebrow[2]=0; //y-coor height
				printf("CALLING - connectedRecursiveCount()\n");
				connectedRecursiveCount(tempImg,loopi,loopj,x0,y0,x1,y1);
				// printf("COUNT::%d",GBL_eyebrow[0]);
				eyebrow[count][0] = GBL_eyebrow[0];
				eyebrow[count][1] = GBL_eyebrow[1]/GBL_eyebrow[0]; // width
				eyebrow[count][2] = GBL_eyebrow[2]/GBL_eyebrow[0]; // heigth
				count++;
				}

			}
		}
		// determine the largest count and store its avg points
		max_index = 0;
		for(loopi=1;loopi<count;loopi++){
			if(eyebrow[loopi][0]>eyebrow[max_index][0]){
				max_index = loopi;
			}
			else if(eyebrow[loopi][0] == eyebrow[max_index][0]){
				if(eyebrow[loopi][2]>eyebrow[max_index][2]){
					max_index = loopi;
				}

			}
		}
		// check if value is acceptable
		if(eyebrow[max_index][1]<x0 || eyebrow[max_index][1]>x1 || eyebrow[max_index][1]<y0 || eyebrow[max_index][1]>y1){
			GBL_output[frameNum].rbrow[1] = cvPoint(eyebrow[max_index][1],eyebrow[max_index][2]);
		}
		else{
			GBL_output[frameNum].rbrow[1] = cvPoint(0,0);
		}

		// In case both above right failed.. init user input
		if(GBL_output[frameNum].rbrow[0].x == 0 && GBL_output[frameNum].rbrow[0].y ==0
				&& GBL_output[frameNum].rbrow[1].x == 0 && GBL_output[frameNum].rbrow[1].y == 0){

			printf("DEBUG -fineCleanHair():: no right brow detected\n");
			cvCopy(GBL_liveImg,livecopy,NULL);
			cvPutText(livecopy,"right eye brow detect failed, mark them",cvPoint(20,20),&disFont,cvScalar(0,0,0,0));
			cvNamedWindow("Mark right eye brow",0);
			cvResizeWindow("Mark right eye brow", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("Mark right eye brow", livecopy);

			// now get the mouse input

			GBL_mouseMarkCount = 0;
			while(GBL_mouseMarkCount<2){
				cvSetMouseCallback("Mark right eye brow",on_mouse,NULL);
				cvWaitKey(0);

			}
			cvDestroyWindow("Mark right eye brow");

			// now we have co-ordinates, put them in
			for(loopi=0;loopi<2;loopi++){
				GBL_output[frameNum].rbrow[loopi].x = GBL_mouseMark[loopi].x;
				GBL_output[frameNum].rbrow[loopi].y = GBL_mouseMark[loopi].y;
			}
		}
		// in case only one of the eyebrows failed
		else if(GBL_output[frameNum].rbrow[0].x == 0 && GBL_output[frameNum].rbrow[0].y ==0){
			GBL_output[frameNum].rbrow[0] = GBL_output[frameNum].rbrow[1];

		}
		else if(GBL_output[frameNum].rbrow[1].x == 0 && GBL_output[frameNum].rbrow[1].y ==0){
					GBL_output[frameNum].rbrow[1] = GBL_output[frameNum].rbrow[0];

		}


		/*
		cvRectangle(tempImg,
						cvPoint(GBL_output[frameNum].eyes[0].x-10,topy),
						cvPoint(centerx,(GBL_output[frameNum].eyes[1].y+GBL_output[frameNum].eyes[0].y +GBL_output[frameNum].eyes[1].height)/2),
						CV_RGB(255, 255, 255), 2, 8, 0);


		cvCircle(tempImg,GBL_output[frameNum].lbrow[0],3,cvScalar(255,255,255,0),-1,8,0);
		cvCircle(tempImg,GBL_output[frameNum].lbrow[1],3,cvScalar(255,255,255,0),-1,8,0);

		cvCircle(tempImg,GBL_output[frameNum].rbrow[0],3,cvScalar(255,255,255,0),-1,8,0);
		cvCircle(tempImg,GBL_output[frameNum].rbrow[1],3,cvScalar(255,255,255,0),-1,8,0);
		*/

		cvOr(GBL_originalImg[frameNum],GBL_maskImg[frameNum],GBL_liveImg,NULL);

        if(GBL_verbose){

            cvNamedWindow("hair cleaned",0);
            cvResizeWindow("hair cleaned", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("hair cleaned", GBL_liveImg);
            cvWaitKey(0);
            cvDestroyWindow("hair cleaned");

            cvNamedWindow("eyeBrow_threshold",0);
            cvResizeWindow("eyeBrow_threshold", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("eyeBrow_threshold", imgThreshed);
            cvWaitKey(0);
            cvDestroyWindow("eyeBrow_threshold");


            /*
            cvNamedWindow("ROI test",0);
            cvResizeWindow("ROI test", DISPLAY_WIDTH,DISPLAY_HEIGHT);
            cvShowImage("ROI test", tempImg);
            cvWaitKey(0);
            cvDestroyWindow("ROI test");
            */
        }
	}


	void cheekDetect(int frameNum){
		// detect the markers on cheek. init user actino if not found

		printf("CALL - cheekDetect()\n");

		//local vars

		IplImage *imgThreshed,*tempImg,*livecopy ;
		int loopi,loopj;
		int eyebrow[10][3];
		CvFont disFont;

		//-------------------------  code logic ---------------------------

		cvInitFont(&disFont,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC,0.5,0.5,0.0,1,8);

		livecopy = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		imgThreshed = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), 8, 1);
		cvInRangeS(GBL_liveImg, cvScalar(100, 0, 0,0), cvScalar(255, 100,100,0), imgThreshed); //YIPPEEEE working

		// to detect and obtain the co-ordinates of cheek marker on both sides
		int topy = (GBL_output[frameNum].eyes[0].y +GBL_output[frameNum].eyes[1].y + GBL_output[frameNum].eyes[0].height + GBL_output[frameNum].eyes[1].height )/2;
		topy = GBL_output[frameNum].face.y + (topy - GBL_output[frameNum].face.y)*0.50;

		int centerx = (GBL_output[frameNum].eyes[0].x +GBL_output[frameNum].eyes[1].x +GBL_output[frameNum].eyes[0].width)/2;
		//left cheek
		tempImg = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), 8, 1); //need this to be a copy of imgThres
		cvCopyImage(imgThreshed,tempImg);
		int count=0;
		int x0 = GBL_output[frameNum].face.x;
		int x1 = centerx;
		int y0 = topy;
		int y1  =  GBL_output[frameNum].mouth.y+GBL_output[frameNum].mouth.height;
		for(loopi=y0;loopi<y1;loopi++){
			for(loopj =x0;loopj<x1;loopj++ ){
				if((((uchar *)(tempImg->imageData + loopi*tempImg->widthStep))[loopj])==255){
				// get the largest connected component in this area
					GBL_eyebrow[0]=0; //count
					GBL_eyebrow[1]=0; // x-coor width
					GBL_eyebrow[2]=0; //y-coor height
				printf("CALLING - connectedRecursiveCount()\n");
				connectedRecursiveCount(tempImg,loopi,loopj,x0,y0,x1,y1);
				// printf("COUNT::%d",GBL_eyebrow[0]);
				eyebrow[count][0] = GBL_eyebrow[0];
				eyebrow[count][1] = GBL_eyebrow[1]/GBL_eyebrow[0]; // width
				eyebrow[count][2] = GBL_eyebrow[2]/GBL_eyebrow[0]; // heigth
				count++;
				}

			}
		}
		// determine the largest count and store its avg points
		int max_index = 0;
		for(loopi=1;loopi<count;loopi++){
			if(eyebrow[loopi][0]>eyebrow[max_index][0]){
				max_index = loopi;
			}
			else if(eyebrow[loopi][0] == eyebrow[max_index][0]){
				if(eyebrow[loopi][2]>eyebrow[max_index][2]){
					max_index = loopi;
				}

			}
		}
		if(eyebrow[max_index][1]<x0 || eyebrow[max_index][1]>x1 || eyebrow[max_index][1]<y0 || eyebrow[max_index][1]>y1){
			GBL_output[frameNum].cheek[0] = cvPoint(eyebrow[max_index][1],eyebrow[max_index][2]);
		}
		else{
			GBL_output[frameNum].cheek[0] = cvPoint(0,0);
		}

		// In case right cheek failed.. init user input
		if(GBL_output[frameNum].cheek[0].x == 0 && GBL_output[frameNum].cheek[0].y ==0){

			printf("DEBUG -fineCleanHair():: no left cheek detected\n");
			cvCopy(GBL_liveImg,livecopy,NULL);
			cvPutText(livecopy,"left cheek detect failed, mark them",cvPoint(20,20),&disFont,cvScalar(0,0,0,0));
			cvNamedWindow("Mark left cheek",0);
			cvResizeWindow("Mark left cheek", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("Mark left cheek", livecopy);

			// now get the mouse input

			GBL_mouseMarkCount = 0;
			while(GBL_mouseMarkCount<1){
				cvSetMouseCallback("Mark left cheek",on_mouse,NULL);
				cvWaitKey(0);

			}
			cvDestroyWindow("Mark left cheek");

			// now we have co-ordinates, put them in
			for(loopi=0;loopi<1;loopi++){
				GBL_output[frameNum].cheek[0].x = GBL_mouseMark[loopi].x;
				GBL_output[frameNum].cheek[0].y = GBL_mouseMark[loopi].y;
			}
		}


		// right cheek
		count=0;
		x0 = x1;
		x1 = GBL_output[frameNum].face.x + GBL_output[frameNum].face.width;
		y0 = topy;
		y1  = GBL_output[frameNum].mouth.y+GBL_output[frameNum].mouth.height;
		for(loopi=y0;loopi<y1;loopi++){
			for(loopj =x0;loopj<x1;loopj++ ){
				if((((uchar *)(tempImg->imageData + loopi*tempImg->widthStep))[loopj])==255){
				// get the largest connected component in this area
					GBL_eyebrow[0]=0; //count
					GBL_eyebrow[1]=0; // x-coor width
					GBL_eyebrow[2]=0; //y-coor height
				printf("CALLING - connectedRecursiveCount()\n");
				connectedRecursiveCount(tempImg,loopi,loopj,x0,y0,x1,y1);
				// printf("COUNT::%d",GBL_eyebrow[0]);
				eyebrow[count][0] = GBL_eyebrow[0];
				eyebrow[count][1] = GBL_eyebrow[1]/GBL_eyebrow[0]; // width
				eyebrow[count][2] = GBL_eyebrow[2]/GBL_eyebrow[0]; // heigth
				count++;
				}

			}
		}
		// determine the largest count and store its avg points
		max_index = 0;
		for(loopi=1;loopi<count;loopi++){
			if(eyebrow[loopi][0]>eyebrow[max_index][0]){
				max_index = loopi;
			}
			else if(eyebrow[loopi][0] == eyebrow[max_index][0]){
				if(eyebrow[loopi][2]>eyebrow[max_index][2]){
					max_index = loopi;
				}

			}
		}
		// check if value is acceptable
		if(eyebrow[max_index][1]<x0 || eyebrow[max_index][1]>x1 || eyebrow[max_index][1]<y0 || eyebrow[max_index][1]>y1){
			GBL_output[frameNum].cheek[1] = cvPoint(eyebrow[max_index][1],eyebrow[max_index][2]);
		}
		else{
			GBL_output[frameNum].cheek[1] = cvPoint(0,0);
		}


		// In case right cheek failed.. init user input
		if(GBL_output[frameNum].cheek[1].x == 0 && GBL_output[frameNum].cheek[1].y ==0){

			printf("DEBUG -fineCleanHair():: no right cheek detected\n");
			cvCopy(GBL_liveImg,livecopy,NULL);
			cvPutText(livecopy,"right cheek detect failed, mark them",cvPoint(20,20),&disFont,cvScalar(0,0,0,0));
			cvNamedWindow("Mark right cheek",0);
			cvResizeWindow("Mark right cheek", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("Mark right cheek", livecopy);

			// now get the mouse input

			GBL_mouseMarkCount = 0;
			while(GBL_mouseMarkCount<1){
				cvSetMouseCallback("Mark right cheek",on_mouse,NULL);
				cvWaitKey(0);

			}
			cvDestroyWindow("Mark right cheek");

			// now we have co-ordinates, put them in
			for(loopi=0;loopi<1;loopi++){
				GBL_output[frameNum].cheek[1].x = GBL_mouseMark[loopi].x;
				GBL_output[frameNum].cheek[1].y = GBL_mouseMark[loopi].y;
			}
		}


		cvCircle(tempImg,GBL_output[frameNum].cheek[0],3,cvScalar(255,255,255,0),-1,8,0);
		cvCircle(tempImg,GBL_output[frameNum].cheek[1],3,cvScalar(255,255,255,0),-1,8,0);

		if(GBL_verbose){

            cvNamedWindow("cheek_mark",0);
			cvResizeWindow("cheek_mark", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("cheek_mark", tempImg);
			cvWaitKey(0);
			cvDestroyWindow("cheek_mark");



			cvNamedWindow("cheek_detect",0);
			cvResizeWindow("cheek_detect", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("cheek_detect", imgThreshed);
			cvWaitKey(0);
			cvDestroyWindow("cheek_detect");

		}

	}


    void cleanBeard(int frameNum){

        IplImage *tempImg,*imgThreshed;
        int loopi,loopj;

    	tempImg = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
		imgThreshed = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), 8, 1);


		// set the threshold, then normalize the imgThreshed
		cvCopyImage(GBL_liveImg,tempImg);
		cvInRangeS(tempImg, cvScalar(0, 0, 0,0), cvScalar(50, 50, 50,0), imgThreshed);

		// now to remove parts that are not probably eyebrows. get a hair point above the ROI of eyebrow. Then remove all connected components
		// of this
/*
			cvNamedWindow("thres set",0);
			cvResizeWindow("thres set", DISPLAY_WIDTH,DISPLAY_HEIGHT);
			cvShowImage("thres set", imgThreshed);
			cvWaitKey(0);
			cvDestroyWindow("thres set");
*/

        int x0 = GBL_output[frameNum].nose.x - 10;
        int x1 = x0 +GBL_output[frameNum].nose.width+20;

        int y0 = GBL_output[frameNum].nose.y+GBL_output[frameNum].nose.height/1.5;
        int y1 = y0 + GBL_output[frameNum].face.y + GBL_output[frameNum].face.height - GBL_output[frameNum].nose.y - GBL_output[frameNum].nose.height/1.5;

		for(loopi=y0;loopi<y1;loopi++){
			for(loopj=x0;loopj<x1;loopj++){
				if((((uchar *)(imgThreshed->imageData + loopi*imgThreshed->widthStep))[loopj])==255){
					// this point is white.. so call the recursive fun
					printf("CALLING - connectedRecursive()\n");
					connectedRecursive(imgThreshed,loopi,loopj,frameNum);

				}
			}
		}
			cvOr(tempImg,GBL_maskImg[frameNum],GBL_liveImg,NULL);




    }

	void lipContour(int frameNum){
		// used to detect lip contour...

		printf("CALL - lipContour()\n");

		//local vars
		IplImage *grayImg,*cannyImg,*grayImg2,*tempImg;
		CvRect rectROI;
		int eyebrow[40][3];

		//loop vars
		int loopi,loopj;

		//-------------- code logic -----------------------------

		// do both connected component length and the amount of black

		// convert the ROI to gray scale and copy to grayImg
		rectROI = cvRect(GBL_output[frameNum].nose.x - 10,GBL_output[frameNum].nose.y+GBL_output[frameNum].nose.height/1.5,GBL_output[frameNum].nose.width+20,GBL_output[frameNum].face.y + GBL_output[frameNum].face.height - GBL_output[frameNum].nose.y - GBL_output[frameNum].nose.height/1.5);
		grayImg = cvCreateImage( cvSize(rectROI.width,rectROI.height), IPL_DEPTH_8U, 1 );
		grayImg2 = cvCreateImage( cvSize(rectROI.width,rectROI.height), IPL_DEPTH_8U, 1 );
		cannyImg = cvCreateImage( cvSize(rectROI.width,rectROI.height), IPL_DEPTH_8U, 1 );
		tempImg = cvCreateImage(cvSize(rectROI.width,rectROI.height), 8, 1); //need this to be a copy of imgThres
		cvSetImageROI(GBL_liveImg,rectROI);
		cvCvtColor( GBL_liveImg, grayImg, CV_BGR2GRAY );
		cvCvtColor( GBL_liveImg, grayImg2, CV_BGR2GRAY );
		cvResetImageROI(GBL_liveImg);

		// the actual canny edge call
		cvCanny(grayImg, cannyImg, 100, 200, 3);

		cvOr(grayImg,cannyImg,grayImg2,NULL);

		// remove 3 contiguous lines of vertical white in grayImg2
		for(loopj=0;loopj<grayImg2->width;loopj++){
			for(loopi=0;loopi<grayImg2->height;loopi++){
				if((((uchar *)(grayImg2->imageData + loopi*grayImg2->widthStep))[loopj])==255){
					if((((uchar *)(grayImg2->imageData + (loopi+1)*grayImg2->widthStep))[loopj])==255){
						if((((uchar *)(grayImg2->imageData + (loopi+2)*grayImg2->widthStep))[loopj])==255){
						    while((((uchar *)(grayImg2->imageData + loopi*grayImg2->widthStep))[loopj])==255){
							((uchar *)(grayImg2->imageData + loopi*grayImg2->widthStep))[loopj]=000;
							loopi++;
						    }
							//((uchar *)(grayImg2->imageData + (loopi+1)*grayImg2->widthStep))[loopj]=000;
							//((uchar *)(grayImg2->imageData + (loopi+2)*grayImg2->widthStep))[loopj]=000;
							//loopi+=3;
						}
					}
				}
			}
		}

		// next detect the maxlength edge on base image. then get the y co-ordinate..
		if(frameNum == 0){
		// run only for base image

				cvCopyImage(grayImg2,tempImg);
		int count=0;
		for(loopi=0;loopi<tempImg->height;loopi++){
			for(loopj =0;loopj<tempImg->width;loopj++ ){
				if((((uchar *)(tempImg->imageData + loopi*tempImg->widthStep))[loopj])==255){
				// get the largest connected component in this area
					GBL_eyebrow[0]=0; //count
					GBL_eyebrow[1]=0; // x-coor width
					GBL_eyebrow[2]=0; //y-coor height
				printf("CALLING - connectedRecursiveCount()\n");
				eyebrow[count][1] = loopj; // width
				eyebrow[count][2] = loopi; // heigth

				connectedRecursiveCountLip(tempImg,loopi,loopj);
				// printf("COUNT::%d",GBL_eyebrow[0]);
				eyebrow[count][0] = GBL_eyebrow[0];
				eyebrow[count][1] = GBL_eyebrow[1]/GBL_eyebrow[0];
				eyebrow[count][2] = GBL_eyebrow[2]/GBL_eyebrow[0];
				count++;
				}

			}
		}

		int max_index = 0;
		for(loopi=1;loopi<count;loopi++){
			if(eyebrow[loopi][0]>eyebrow[max_index][0]){
				max_index = loopi;
			}
			else if(eyebrow[loopi][0] == eyebrow[max_index][0]){
				if(eyebrow[loopi][2]>eyebrow[max_index][2]){
					max_index = loopi;
				}

			}
		}

		GBL_lipEdgex = eyebrow[max_index][1];
		GBL_lipEdgey = eyebrow[max_index][2];
        GBL_output[0].lipCount = eyebrow[max_index][0];
        // test case circle
       // cvCircle(grayImg2,cvPoint(GBL_lipEdgex,GBL_lipEdgey),4,cvScalar(0,0,0,0),-1,8,0);

		}
		else{
		    int count = 0;
     		for(loopi=GBL_lipEdgey-10;loopi<GBL_lipEdgey+10;loopi++){
                for(loopj =10;loopj<grayImg2->width - 10;loopj++ ){
                    if((((uchar *)(grayImg2->imageData + loopi*grayImg2->widthStep))[loopj])==255){
                        count++;
                    }

                }
            }
        GBL_output[frameNum].lipCount = count;

		}

		cvRectangle(grayImg2,
							cvPoint(10, GBL_lipEdgey-10),
							cvPoint(grayImg2->width - 10, GBL_lipEdgey+10),
							CV_RGB(255, 255, 255), 1, 8, 0);

		if(GBL_verbose){
			//cvNamedWindow("canny",0);
			cvNamedWindow("gray",0);
			//cvShowImage( "canny", cannyImg );
			cvShowImage( "gray", grayImg2 );

			cvWaitKey(0);
			//cvDestroyWindow("canny");
			cvDestroyWindow("gray");
			cvReleaseImage( &grayImg );
			cvReleaseImage( &cannyImg );
		}
		return;


	}

	int imageProcess(){
		//local vars
		//loop vars
		int loopi = 0;
		//-------------- code logic -----------------------------

		// for each frame/Image we have Do image processing
		for(loopi=0;loopi<GBL_frameCount;loopi++){


			// cpy to live image for manupilation
			GBL_liveImg = GBL_originalImg[loopi];

			// show the live unmodified image before proceeding
                cvNamedWindow("Original Image",0);
                cvResizeWindow("Original Image", DISPLAY_WIDTH,DISPLAY_HEIGHT);
                cvShowImage("Original Image", GBL_liveImg);
                cvWaitKey(0);
                cvDestroyWindow("Original Image");

			// create maskImg[loopi]
			/* src is the source image which you want to mask
			 * mask is a single channel binary image as a mask
			 * result is the image with the same size, depth, channel with src
			 */
			GBL_maskImg[loopi] = cvCreateImage(cvSize(GBL_liveImg->width, GBL_liveImg->height), GBL_liveImg->depth, GBL_liveImg->nChannels);
			// set to black color
			cvZero(GBL_maskImg[loopi]);

			// set the image param in output
			GBL_output[loopi].imgHeight = GBL_liveImg->height;
			GBL_output[loopi].imgWidth = GBL_liveImg->width;

			GBL_output[loopi].isError = 0;
			// call face detect, check if face detected, else exit as critical error
			faceDetect(loopi);
			if(GBL_output[loopi].isFace == 0){
				fprintf(stderr,"ERROR - imageProcess()::Critical error. No face detected in given frame: %d\n",loopi+1);
				exitFunction(-1);
			}
			else{
				printf("DEBUG - imageProcess():: face detected from faceDetect()\n");
				// before displaying check for verbose
				if(GBL_verbose==1){
					cvNamedWindow("Face Detect Ellipse",0);
					cvResizeWindow("Face Detect Ellipse", DISPLAY_WIDTH,DISPLAY_HEIGHT);
					cvShowImage("Face Detect Ellipse", GBL_liveImg);
					cvWaitKey(0);
					cvDestroyWindow("Face Detect Ellipse");


					cvNamedWindow("Mask Image",0);
					cvResizeWindow("Mask Image", DISPLAY_WIDTH,DISPLAY_HEIGHT);
					cvShowImage("Mask Image", GBL_maskImg[loopi]);
					cvWaitKey(0);
					cvDestroyWindow("Mask Image");

				}

			}
			// wont return anything. because the errors are soft.and are handled in the function.
			eyeMouthNoseDetect(loopi);

			// Lip contour detection TODO :: when to call this? calling here/end has no diff
			//lipContour(loopi);


			// have to also pass step5,isIll, is Mirror
			faceSegment(loopi,1,0,1);

			//finely clean hair and detect eyebrows
			fineCleanHair(loopi);

			// detect cheek markers
			cheekDetect(loopi);
			printf("EXIT cheekDetect()\n");

			// Lip contour detection TODO :: when to call this?
			//beard/moustache removal
			// do only for base image
			cleanBeard(loopi);

			lipContour(loopi);

            // has been put here for 2 reasons, 1 help understands its role in canny, 2 is the final liveimg
            if(GBL_verbose){
                cvNamedWindow("beard remove",0);
                cvResizeWindow("beard remove", DISPLAY_WIDTH,DISPLAY_HEIGHT);
                cvShowImage("beard remove", GBL_liveImg);
                cvWaitKey(0);
                cvDestroyWindow("beard remove");
            }

			//for testing
            /*
				cvNamedWindow("Final Image",0);
				cvResizeWindow("Final Image", DISPLAY_WIDTH,DISPLAY_HEIGHT);
				cvShowImage("Final Image", GBL_liveImg);
				cvWaitKey(0);
				cvDestroyWindow("Final Image");
            */
		}

		return 0;
	}

	void outputProcess(){

	    // incomplete function so return
	    //return;
		// make sense, display it and output final results
		printf("CALL - outputProcess()");

        //local vars
        int refEyeLength = 50;
        int isValid;

        struct outData{
            int eyeLength;
            int centerx,centery;
            //int eyex[2];
            //int eyey[2];
            double mouthx[2];
            double mouthy[2];
            double lbrowx[2];
            double lbrowy[2];
            double rbrowx[2];
            double rbrowy[2];
            double cheekx[2];
            double cheeky[2];
            int lipcount;

        }data[10];

        struct dataDiff{

            double mouthx[2];
            double mouthy[2];
            double lbrowx[2];
            double lbrowy[2];
            double rbrowx[2];
            double rbrowy[2];
            double cheekx[2];
            double cheeky[2];
            int lipcount;
        }diff[9];

        struct normalData{

            double mouthx[2];
            double mouthy[2];
            double lbrowx[2];
            double lbrowy[2];
            double rbrowx[2];
            double rbrowy[2];
            double cheekx[2];
            double cheeky[2];
            double lipcount;

        }normal[9];

        FILE *fp;
        char *filename = "/tmp/emorecfile";

        double mulFac=0.00;
        //loop vars
        int loopi;

        //code logic
        fp = fopen(filename,"w");
        if(!fp){
        fprintf(stderr,"ERROR - outputProcess()::File open failed\n");
        return;
        }


        for(loopi=0;loopi<GBL_frameCount;loopi++){
        // for each frame transform all co-ordinates to the center of eyes
        data[loopi].eyeLength = GBL_output[loopi].eyes[1].x - GBL_output[loopi].eyes[0].x;
        mulFac = (double)refEyeLength/ (double)data[loopi].eyeLength ;

        data[loopi].centerx = (GBL_output[loopi].eyes[1].x + GBL_output[loopi].eyes[0].x)/2;
        data[loopi].centery = (GBL_output[loopi].eyes[1].y + GBL_output[loopi].eyes[0].y)/2;

        data[loopi].mouthx[0] = GBL_output[loopi].mouth.x -  data[loopi].centerx;
        data[loopi].mouthx[1] = GBL_output[loopi].mouth.x + GBL_output[loopi].mouth.width -  data[loopi].centerx;
        data[loopi].mouthx[0] *= mulFac;
        data[loopi].mouthx[1] *= mulFac;

        data[loopi].mouthy[0] = GBL_output[loopi].mouth.y -  data[loopi].centery;
        data[loopi].mouthy[1] = GBL_output[loopi].mouth.y + GBL_output[loopi].mouth.height -  data[loopi].centery;
        data[loopi].mouthy[0] *= mulFac;
        data[loopi].mouthy[1] *= mulFac;


        data[loopi].lbrowx[0] = GBL_output[loopi].lbrow[0].x -  data[loopi].centerx;
        data[loopi].lbrowx[1] = GBL_output[loopi].lbrow[1].x -  data[loopi].centerx;
        data[loopi].lbrowx[0] *= mulFac;
        data[loopi].lbrowx[1] *= mulFac;


        data[loopi].lbrowy[0] = GBL_output[loopi].lbrow[0].y -  data[loopi].centery;
        data[loopi].lbrowy[1] = GBL_output[loopi].lbrow[1].y -  data[loopi].centery;
        data[loopi].lbrowy[0] *= mulFac;
        data[loopi].lbrowy[1] *= mulFac;

        data[loopi].rbrowx[0] = GBL_output[loopi].rbrow[0].x - data[loopi].centerx;
        data[loopi].rbrowx[1] = GBL_output[loopi].rbrow[1].x - data[loopi].centerx;
        data[loopi].rbrowx[0] *= mulFac;
        data[loopi].rbrowx[1] *= mulFac;

        data[loopi].rbrowy[0] = GBL_output[loopi].rbrow[0].y - data[loopi].centery;
        data[loopi].rbrowy[1] = GBL_output[loopi].rbrow[1].y - data[loopi].centery;
        data[loopi].rbrowy[0] *= mulFac;
        data[loopi].rbrowy[1] *= mulFac;


        data[loopi].cheekx[0] = GBL_output[loopi].cheek[0].x - data[loopi].centerx;
        data[loopi].cheekx[1] = GBL_output[loopi].cheek[1].x - data[loopi].centerx;
        data[loopi].cheekx[0] *= mulFac;
        data[loopi].cheekx[1] *= mulFac;


        data[loopi].cheeky[0] = GBL_output[loopi].cheek[0].y - data[loopi].centery;
        data[loopi].cheeky[1] = GBL_output[loopi].cheek[1].y - data[loopi].centery;
        data[loopi].cheeky[0] *= mulFac;
        data[loopi].cheeky[1] *= mulFac;

        data[loopi].lipcount = GBL_output[loopi].lipCount;

        }

        for(loopi=1;loopi<GBL_frameCount;loopi++){

            diff[loopi].mouthx[0] = data[loopi].mouthx[0] - data[0].mouthx[0];
            diff[loopi].mouthx[1] = data[loopi].mouthx[1] - data[0].mouthx[1];

            diff[loopi].mouthy[0] = data[loopi].mouthy[0] - data[0].mouthy[0];
            diff[loopi].mouthy[1] = data[loopi].mouthy[1] - data[0].mouthy[1];

            diff[loopi].lbrowx[0] = data[loopi].lbrowx[0] - data[0].lbrowx[0];
            diff[loopi].lbrowx[1] = data[loopi].lbrowx[1] - data[0].lbrowx[1];

            diff[loopi].lbrowy[0] = data[loopi].lbrowy[0] - data[0].lbrowy[0];
            diff[loopi].lbrowy[1] = data[loopi].lbrowy[1] - data[0].lbrowy[1];

            diff[loopi].rbrowx[0] = data[loopi].rbrowx[0] - data[0].rbrowx[0];
            diff[loopi].rbrowx[1] = data[loopi].rbrowx[1] - data[0].rbrowx[1];

            diff[loopi].rbrowy[0] = data[loopi].rbrowy[0] - data[0].rbrowy[0];
            diff[loopi].rbrowy[1] = data[loopi].rbrowy[1] - data[0].rbrowy[1];

            diff[loopi].cheekx[0] = data[loopi].cheekx[0] - data[0].cheekx[0];
            diff[loopi].cheekx[1] = data[loopi].cheekx[1] - data[0].cheekx[1];

            diff[loopi].cheeky[0] = data[loopi].cheeky[0] - data[0].cheeky[0];
            diff[loopi].cheeky[1] = data[loopi].cheeky[1] - data[0].cheeky[1];


             diff[loopi].lipcount = data[loopi].lipcount - data[0].lipcount;
        }


          for(loopi=1;loopi<GBL_frameCount;loopi++){

                normal[loopi].mouthx[0] = diff[loopi].mouthx[0] / 25;
                normal[loopi].mouthx[1] = diff[loopi].mouthx[1] / 25;

                normal[loopi].mouthy[0] = diff[loopi].mouthy[0] / 25;
                normal[loopi].mouthy[1] = diff[loopi].mouthy[1] / 25;


                normal[loopi].lbrowx[0] = diff[loopi].lbrowx[0] / 25;
                normal[loopi].lbrowx[1] = diff[loopi].lbrowx[1] / 25;

                normal[loopi].lbrowy[0] = diff[loopi].lbrowy[0] / 25;
                normal[loopi].lbrowy[1] = diff[loopi].lbrowy[1] / 25;

                normal[loopi].rbrowx[0] = diff[loopi].rbrowx[0] / 25;
                normal[loopi].rbrowx[1] = diff[loopi].rbrowx[1] / 25;

                normal[loopi].rbrowy[0] = diff[loopi].rbrowy[0] / 25;
                normal[loopi].rbrowy[1] = diff[loopi].rbrowy[1] / 25;


                normal[loopi].cheekx[0] = diff[loopi].cheekx[0] / 25;
                normal[loopi].cheekx[1] = diff[loopi].cheekx[1] / 25;

                normal[loopi].cheeky[0] = diff[loopi].cheeky[0] / 25;
                normal[loopi].cheeky[1] = diff[loopi].cheeky[1] / 25;

                normal[loopi].lipcount = (double) diff[loopi].lipcount / (double)(50*20);

                 normal[loopi].mouthx[0] = fabs( normal[loopi].mouthx[0]);
                 normal[loopi].mouthx[1] = fabs( normal[loopi].mouthx[1]);

                 normal[loopi].mouthy[0] = fabs( normal[loopi].mouthy[0]);
                 normal[loopi].mouthy[1] = fabs( normal[loopi].mouthy[1]);

                 normal[loopi].lbrowx[0] = fabs( normal[loopi].lbrowx[0]);
                 normal[loopi].lbrowx[1] = fabs( normal[loopi].lbrowx[1]);

                 normal[loopi].lbrowy[0] = fabs( normal[loopi].lbrowy[0]);
                 normal[loopi].lbrowy[1] = fabs( normal[loopi].lbrowy[1]);

                 normal[loopi].rbrowx[0] = fabs( normal[loopi].rbrowx[0]);
                 normal[loopi].rbrowx[1] = fabs( normal[loopi].rbrowx[1]);

                 normal[loopi].rbrowy[0] = fabs( normal[loopi].rbrowy[0]);
                 normal[loopi].rbrowy[1] = fabs( normal[loopi].rbrowy[1]);

                 normal[loopi].cheekx[0] = fabs( normal[loopi].cheekx[0]);
                 normal[loopi].cheekx[1] = fabs( normal[loopi].cheekx[1]);

                 normal[loopi].cheeky[0] = fabs( normal[loopi].cheeky[0]);
                 normal[loopi].cheeky[1] = fabs( normal[loopi].cheeky[1]);

                  normal[loopi].lipcount = fabs(  normal[loopi].lipcount);






          }




        printf("DEBUG - outputProcess():: printing all outputs\n");

        for(loopi=0;loopi<GBL_frameCount;loopi++){
             printf("Out Put for %d\n###########################################\n",loopi);
             printf("eyeLength  %d\n",data[loopi].eyeLength);
             printf("centerx  %d\n",data[loopi].centerx);
             printf("centery  %d\n",data[loopi].centery);


             printf("mouthx0  %G\n",data[loopi].mouthx[0]);
             printf("mouthx1 %G\n",data[loopi].mouthx[1]);

             printf("mouthy0  %G\n",data[loopi].mouthy[0]);
             printf("mouthy1  %G\n",data[loopi].mouthy[1]);

             printf("lbrowx0  %G\n",data[loopi].lbrowx[0]);
             printf("lbrowx1  %G\n",data[loopi].lbrowx[1]);

             printf("lbrowy0  %G\n",data[loopi].lbrowy[0]);
             printf("lbrowy1  %G\n",data[loopi].lbrowy[1]);

             printf("rbrowx0  %G\n",data[loopi].rbrowx[0]);
             printf("rbrowx1  %G\n",data[loopi].rbrowx[1]);

             printf("rbrowy0  %G\n",data[loopi].rbrowy[0]);
             printf("rbrowy1 %G\n",data[loopi].rbrowy[1]);

             printf("cheekx0  %G\n",data[loopi].cheekx[0]);
             printf("cheekx1  %G\n",data[loopi].cheekx[1]);

             printf("cheeky0  %G\n",data[loopi].cheeky[0]);
             printf("cheeky1  %G\n",data[loopi].cheeky[1]);

             printf("lipcount  %d\n",data[loopi].lipcount);

             printf("\n####################################\n\n\n");



        }


        // print diffs
       for(loopi=1;loopi<GBL_frameCount;loopi++){
             printf("DIFF for %d\n---------------------------------------------------\n",loopi);

             printf("mouthx0  %G\n",diff[loopi].mouthx[0]);
             printf("mouthx1  %G\n",diff[loopi].mouthx[1]);

             printf("mouthy0  %G\n",diff[loopi].mouthy[0]);
             printf("mouthy1  %G\n",diff[loopi].mouthy[1]);

             printf("lbrowx0  %G\n",diff[loopi].lbrowx[0]);
             printf("lbrowx1  %G\n",diff[loopi].lbrowx[1]);

             printf("lbrowy0  %G\n",diff[loopi].lbrowy[0]);
             printf("lbrowy1  %G\n",diff[loopi].lbrowy[1]);

             printf("rbrowx0  %G\n",diff[loopi].rbrowx[0]);
             printf("rbrowx1  %G\n",diff[loopi].rbrowx[1]);

             printf("rbrowy0  %G\n",diff[loopi].rbrowy[0]);
             printf("rbrowy1  %G\n",diff[loopi].rbrowy[1]);

             printf("cheekx0  %G\n",diff[loopi].cheekx[0]);
             printf("cheekx1  %G\n",diff[loopi].cheekx[1]);

             printf("cheeky0  %G\n",diff[loopi].cheeky[0]);
             printf("cheeky1  %G\n",diff[loopi].cheeky[1]);

             printf("lipcount  %d\n",diff[loopi].lipcount);

             printf("\n---------------------------------------------------\n\n\n");
        }


    for(loopi=1;loopi<GBL_frameCount;loopi++){
             printf("normal for %d\n---------------------------------------------------\n",loopi);

             printf("mouthx0  %G\n",normal[loopi].mouthx[0]);
             printf("mouthx1  %G\n",normal[loopi].mouthx[1]);

             printf("mouthy0  %G\n",normal[loopi].mouthy[0]);
             printf("mouthy1  %G\n",normal[loopi].mouthy[1]);

             printf("lbrowx0  %G\n",normal[loopi].lbrowx[0]);
             printf("lbrowx1  %G\n",normal[loopi].lbrowx[1]);

             printf("lbrowy0  %G\n",normal[loopi].lbrowy[0]);
             printf("lbrowy1  %G\n",normal[loopi].lbrowy[1]);

             printf("rbrowx0  %G\n",normal[loopi].rbrowx[0]);
             printf("rbrowx1  %G\n",normal[loopi].rbrowx[1]);

             printf("rbrowy0  %G\n",normal[loopi].rbrowy[0]);
             printf("rbrowy1  %G\n",normal[loopi].rbrowy[1]);

             printf("cheekx0  %G\n",normal[loopi].cheekx[0]);
             printf("cheekx1  %G\n",normal[loopi].cheekx[1]);

             printf("cheeky0  %G\n",normal[loopi].cheeky[0]);
             printf("cheeky1  %G\n",normal[loopi].cheeky[1]);

             printf("lipcount  %G\n",normal[loopi].lipcount);

             printf("\n---------------------------------------------------\n\n\n");
        }

/*for batch files
printf("Is this valid data? 1-yes.... 0-no\n");
scanf("%d",&isValid);


if(!isValid){
fprintf(fp, " ");

fclose(fp);


}



*/


       for(loopi=1;loopi<GBL_frameCount;loopi++){


             fprintf(fp, "%G ",normal[loopi].mouthx[0]);
             fprintf(fp, "%G ",normal[loopi].mouthx[1]);

             fprintf(fp, "%G ",normal[loopi].mouthy[0]);
             fprintf(fp, "%G ",normal[loopi].mouthy[1]);

             fprintf(fp, "%G ",normal[loopi].lbrowx[0]);
             fprintf(fp, "%G ",normal[loopi].lbrowx[1]);

             fprintf(fp, "%G ",normal[loopi].lbrowy[0]);
             fprintf(fp, "%G ",normal[loopi].lbrowy[1]);

             fprintf(fp, "%G ",normal[loopi].rbrowx[0]);
             fprintf(fp, "%G ",normal[loopi].rbrowx[1]);

             fprintf(fp, "%G ",normal[loopi].rbrowy[0]);
             fprintf(fp, "%G ",normal[loopi].rbrowy[1]);

             fprintf(fp, "%G ",normal[loopi].cheekx[0]);
             fprintf(fp, "%G ",normal[loopi].cheekx[1]);

             fprintf(fp, "%G ",normal[loopi].cheeky[0]);
             fprintf(fp, "%G ",normal[loopi].cheeky[1]);
            fprintf(fp, "%G ",normal[loopi].lipcount);
             fprintf(fp,"\n");

        }

fclose(fp);


/*
        printf("Enter 0-mouth closed,1 - mouth open\n");
        scanf("%d"&mouth);

        // few calculations and displaying for the base image
        eyeLengthBase =  GBL_output[0].eyes[1].x - GBL_output[0].eyes[0].x;

        for(loopi=1;loopi<GBL_frameCount;loopi++){
            eyeLength = GBL_output[loopi].eyes[1].x - GBL_output[loopi].eyes[0].x;
            // 0 is x, 1 is y
            for(loopj=0;loopj<4;loopj++){
                eyebrowChange[0][0] = GBL_output[loopi].lbrow[0].x - GBL_output[0].lbrow[0].x;
                eyebrowChange[0][1] = GBL_output[loopi].lbrow[0].y - GBL_output[0].lbrow[0].y;
            }

        }
*/
        return;
	}



	int copyRect(CvRect *src, CvRect * dest){

		dest->x = src->x;
		dest->y = src->y;
		dest->width = src->width;
		dest->height = src->height;

		return 0;
	}
