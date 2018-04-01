#include<opencv2/opencv.hpp>
#include"PersonTracking.hpp"
using namespace std;
using namespace cv;
#define devView(i) imshow(#i,i)

int main()
{
	PersonTracking person;
   person.KillDCM();
	person.Init();
	for (; waitKey(1) != 27; )
	{
		Mat show = person.QueryColorImage();
		if (person.Update())
		{
			auto Tperson = person.Queryperson();
			//cout << person.QueryBoundingBox(Tperson) << endl;
			show = person.draw(Tperson);
			cout << person.QueryMassCenterWorld(Tperson).x << "  "
			<< person.QueryMassCenterWorld(Tperson).y << "   "
			<<person.QueryMassCenterWorld(Tperson).z<<endl;
		}
		devView(show);
		person.release();
	}
	return 0;
}