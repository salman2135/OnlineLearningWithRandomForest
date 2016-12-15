
#include <opencv2/core.hpp>
#include <opencv2/core/utility.hpp>
#include <opencv2/tracking/tracker.hpp>
#include <opencv2/highgui.hpp>
#include "opencv2/imgcodecs.hpp"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <boost/timer/timer.hpp>



using namespace std;
using namespace cv;

static cv::Mat Image;
static bool is_selection_started = false;
static bool is_selection_done = false;
static cv::Rect2d BBox;
static std::string WindowName = "Median FLow";

void RunTracking(Rect2d inputRectangle);

static void OnMouse(int event, int x, int y, int, void*)
{
	if (!is_selection_done)
	{
		switch (event)
		{
		case cv::EVENT_LBUTTONDOWN:
			//set origin of the bounding box
			is_selection_started = true;
			BBox.x = x;
			BBox.y = y;
			break;
		case cv::EVENT_LBUTTONUP:
			//sei with and height of the bounding box
			BBox.width = std::abs(x - BBox.x);
			BBox.height = std::abs(y - BBox.y);
			is_selection_done = true;
			RunTracking(BBox);
			break;
		case cv::EVENT_MOUSEMOVE:
			if (is_selection_started)
			{
				//draw the bounding box
				cv::Mat currentFrame;
				Image.copyTo(currentFrame);
				cv::rectangle(currentFrame, cv::Point(BBox.x, BBox.y), cv::Point(x, y), cv::Scalar(255, 0, 0), 2, 1);
				cv::imshow(WindowName, currentFrame);
			}
			break;
		}
	}
}

void RunTracking(Rect2d inputRectangle){
			boost::timer::cpu_timer timer;

			cv::String tracker_algorithm = "MEDIANFLOW";


			std::string sequence_path="/home/ss/retinal_dataset/retinal_dataset/";
			int start_frame = 224;
			int end_frame = 625;
			int count = 6;
			std::string ext = "png";
			//cv::Rect2d initial_rectangle(155.00,425.00,50.00,110.00);
			bool is_save_enabled = true ;
			bool is_show_enabled = true;
			std::string tracker_path = "/home/ss/tracker_output/";
			std::string output_image_path = "/home/ss/tracker_output/image/";

			cv::Ptr<cv::Tracker> tracker = cv::Tracker::create(tracker_algorithm);
				if (tracker.empty())
				{
					std::cerr << "***Error in the instantiation of the tracker...***\n";

			}

				cv::Mat frame;
				cv::Rect2d bbox = inputRectangle;
				bool initialized = false;
				bool finished = false;
				int frame_count = 0;
				std::string filename;

				// result files
				std::ofstream result_text;
				result_text.open("/home/ss/tracker_output/result.txt");
				std::ofstream result_fps ("/home/ss/tracker_output/result_fps.txt");


				for (int f_id = start_frame; f_id <= end_frame; ++f_id)
				{
					std::ostringstream oss;
					oss << std::setw(count) << std::setfill('0') << f_id;
					filename = sequence_path + oss.str() + "." + ext;
					frame = cv::imread(filename);


					if (frame.empty())
					{
						std::cerr << "***Could not load a image: " + sequence_path + "/" + oss.str() + "." + ext + "***\n";
						std::cerr << "Current parameter's value: \n";

						return ;
					}
					if (!initialized)
					{

						std::cout << "Tracker initialization...";
						timer.start();
						if (!tracker->init(frame, bbox))
						{
							std::cerr << "***Could not initialize tracker...***\n";
							return ;
						}
						else
						{
							timer.stop();
							std::cout << "Success." << std::endl;
							frame_count++;
							result_text << bbox.x << "," << bbox.y << "," << bbox.width << "," << bbox.height << std::endl;;
							initialized = true;
						}
					}
					else if (finished)
					{
						result_text << "0,0,0,0" << std::endl;
					}
					else
					{
						timer.resume();

						//updates the tracker
						tracker->update(frame, bbox);
						timer.stop();
						if ((bbox.x < 0) || (bbox.y < 0)
								|| (frame.cols < bbox.x+bbox.width) || (frame.rows < bbox.y+bbox.height) )
						{
							// the target is out of sight
							finished = true;
							result_text << "0,0,0,0" << std::endl;
							continue;
						}
						std::cout << "Process: " << filename << std::endl;
						frame_count++;
						result_text << bbox.x << "," << bbox.y << "," << bbox.width << "," << bbox << std::endl;
						if (is_save_enabled || is_show_enabled)
						{
							cv::rectangle(frame, bbox, cv::Scalar(255, 0, 0), 2, 1);
						}
						if (is_show_enabled)
						{
							cv::imshow("Tracking API: " + tracker_algorithm, frame);
							char c = (char)cv::waitKey(2);
							if (c == 'q')
								break;
						}
						if (is_save_enabled)
						{
							cv::imwrite(output_image_path + oss.str() + "." + ext, frame);
						}
					}
				}

				std::cout << "Fin." << std::endl;
				double fps = static_cast<double>(frame_count) / (timer.elapsed().wall / 1000000000.);
				result_fps << fps << std::endl;
				std::cout << "total frames: " << frame_count << std::endl;
				std::cout << "FPS: " << fps << std::endl;

				result_text.close();
				result_fps.close();
}

int main(int argc, char **argv) {


	    Image = imread("/home/ss/retinal_dataset/retinal_dataset/000224.png");   // Read the file

	    if(! Image.data )                              // Check for invalid input
	    {
	        cout <<  "Could not open or find the image" << std::endl ;
	        return -1;
	    }

	    cv::namedWindow(WindowName, 0);
	    cv::setMouseCallback(WindowName, OnMouse, 0);// Create a window for display.
	    imshow( WindowName, Image );

	    waitKey(0);

	    return 0;
}


