#include<opencv2/opencv.hpp>
#include"PersonTracking.hpp"
using namespace std;
using namespace cv;
#define devView(i) imshow(#i,i)

int main()
{
	PersonTracking person;
 //  person.KillDCM();
	person.Init();
	for (; waitKey(15) != 27; )
	{
		
		if (!person.Update()) continue;
		Mat show = person.draw(person.Queryperson());
	    cout << person.QueryMassCenterWorld(person.Queryperson()).x << "  " 
			<< person.QueryMassCenterWorld(person.Queryperson()).y << "   "
			<<person.QueryMassCenterWorld(person.Queryperson()).z<<endl;
		devView(show);
	}
	return 0;
}