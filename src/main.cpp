#include "../includes/CRoadMap.h"

int main(int argc, char **argv) //argv -> .out file + {start keyPointId, end keyPointId}
{
    RoadMap R;

    if(argc == 3)
        R.FindShortestRoute(atol(argv[1]), atol(argv[2]));
    
    return 0;
};