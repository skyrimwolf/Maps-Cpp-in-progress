#include "structs.h"

class RoadMap
{
private:
    std::unordered_map<long, KeyPoint> m_roadPoints;                                                               //{id of keyPoint, whole keyPoint - without way info}
    std::unordered_map<long, Road> m_road;                                                                         //{id of keyPoint, Road struct}
public:
    //init-deinit functions
    RoadMap();
    ~RoadMap();
    
    //helper functions
    std::fstream FileOpener(const char *, std::ios_base::openmode);
    double HaversineFormula(double, double, double, double);
    
    //main functions
    void BuildKeyPoints();
    void BuildRoad();
    void InsertKeys(std::vector<long> &, Way &);
    void FindShortestRoute(long, long);
    void PrintRoute(long, long);
};

//################################################################ INIT-DEINIT FUNCTIONS ########################################################################################

//constructor
RoadMap::RoadMap()
{
    BuildKeyPoints();
    BuildRoad();
}

//destructor - cleanup
RoadMap::~RoadMap()
{
    m_roadPoints.clear();                                                                                            //delete map
    
    for(auto x : m_road)
        x.second.s_keySet.clear();                                                                                   //delete set
    
    m_road.clear();                                                                                                  //delete map
}

//################################################################ HELPER FUNCTIONS #############################################################################################

//opens a file, where howToOpen presents mode in which to open the file (readonly/writeonly, etc..)
std::fstream RoadMap::FileOpener(const char *name, std::ios_base::openmode howToOpen)
{
    std::fstream fd;
    
    fd.open(name, howToOpen);
    if(!fd.is_open())
    {
        std::cout << "Error: File " << name << " could not be opened.\n";
        exit(EXIT_FAILURE);
    };
    
    return fd;
}

//returns distance in meters from one point to another
double RoadMap::HaversineFormula(double latStart, double lonStart, double latEnd, double lonEnd) 
{ 
    double diffLat = (latEnd - latStart) * M_PI/180.0; 
    double diffLon = (lonEnd - lonStart) * M_PI/180.0; 
    double radius  = 6371.0;
    
    latStart = latStart * M_PI/180.0;                                                                               //convert latitude and longitude to radians
    latEnd   = latEnd * M_PI/180.0; 
  
    double a = sin(diffLat/2.0)*sin(diffLat/2.0) + sin(diffLon/2.0)*sin(diffLon/2.0) * cos(latStart)*cos(latEnd);   //haversine formula
    
    return 1000.0 * radius * 2.0 * atan2(sqrt(a), sqrt(1.0-a));                                                     //1000.0 ->convert to meters
}                                                                                                                   //2.0 * atan2(sqrt(a), sqrt(1.0-a))  -> part of the formula

//################################################################ MAIN FUNCTIONS ##############################################################################################

//load all the keyPoints in a vector from KeyPoints.txt
void RoadMap::BuildKeyPoints()
{
    bool isKeyPointsLoaded = true;
    long index = 0;
    std::fstream fKeyPoints = FileOpener("../files/keyPoints.txt", std::fstream::in);
    KeyPoint tmpKeyPoint;
    
    while(isKeyPointsLoaded)
    {
        isKeyPointsLoaded = fKeyPoints.eof() != (bool)(fKeyPoints >> tmpKeyPoint.s_id >> tmpKeyPoint.s_latitude >> tmpKeyPoint.s_longitude);
        if(isKeyPointsLoaded)
        {
            tmpKeyPoint.s_index = index++;
            
            m_roadPoints[tmpKeyPoint.s_id] = tmpKeyPoint;
        };
    };
    
    fKeyPoints.close();                                                                                             //cleanup
}

//build the whole road and a network of keyPoints - true init function
void RoadMap::BuildRoad()
{
    bool isWayLoaded = true;
    std::fstream file     = FileOpener("../files/roads.txt", std::fstream::in);
    std::string tmpString = "NONAME";                                                                               //it will get string "id"
    std::vector<long> tmpVector;
    long tmpId;
    Way tmpWay;

    while(isWayLoaded)                                                                                              //run through entire file and get all the data
    {
        isWayLoaded = isWayLoaded && file.eof() != (bool)(file >> tmpString);
        if(isWayLoaded)
        {
            switch(g_nameTag[tmpString])                                                                            //switch between: way=1, id=2, oneway=3, name=4, operator=5
            {
                case 1:
                    if(tmpWay.s_wayId)                                                                              //if way id is 0, then it's the start of the program
                    {
                        InsertKeys(tmpVector, tmpWay);                                                              //first connect taken data, then reset tmpWay and begin new cycle
                        tmpWay = {0, false, "NONAME"};                                                              //id, oneway, name
                    };
                    
                    isWayLoaded = isWayLoaded && file.eof() != (bool)(file >> tmpWay.s_wayId);                      //get next way id, begining of a new cycle
                    break;
                case 2:
                    isWayLoaded = isWayLoaded && file.eof() != (bool)(file >> tmpId);
                    if(isWayLoaded)
                        tmpVector.emplace_back(tmpId);                                                              //get the keyPointId that will be passed on later in InsertKeys method
                    break;
                case 3:
                    isWayLoaded = isWayLoaded && file.eof() != (bool)(file >> tmpWay.s_oneWay);                     //get the info if its oneway or not
                    break;
                case 4: case 5:
                    isWayLoaded = isWayLoaded && file.eof() != (bool) std::getline(file, tmpWay.s_name);            //get the name of the street
                    break;
            };
        };
    };
    
    file.close();                                                                                                   //cleanup
    tmpVector.clear();                                                                                              //cleanup
    g_nameTag.clear();                                                                                              //cleanup
}

//insert and connect all nodes of one way block
void RoadMap::InsertKeys(std::vector<long> &keyIds, Way &tmpWay)
{
    KeyPoint tmpKeyPoint1 = m_roadPoints[*(keyIds.begin())];                                                        //get the keyPoints through given ids
    
    for(auto it = keyIds.begin(); it != keyIds.end() - 1; it++)                                                     //loop until end()-1 because of *(it+1) in the loop
    {
        KeyPoint &tmpKeyPoint2 = m_roadPoints[*(it+1)];
        double weight          = HaversineFormula(tmpKeyPoint2.s_latitude, tmpKeyPoint2.s_longitude, tmpKeyPoint1.s_latitude, tmpKeyPoint1.s_longitude);
        
        tmpKeyPoint1.s_weight  = weight;
        tmpKeyPoint2.s_weight  = weight; 
        tmpKeyPoint1.s_ways.emplace_back(tmpWay);
        tmpKeyPoint2.s_ways.emplace_back(tmpWay);                                                                   //add weight to keyPoints and way info
        
        m_road[*it].s_id        = tmpKeyPoint1.s_id; 
        m_road[*it].s_index     = tmpKeyPoint1.s_index; 
        m_road[*it].s_keySet.insert(tmpKeyPoint2);                                                                  //add id, index and connections of a given KeyPoint
        m_road[*(it+1)].s_id    = tmpKeyPoint2.s_id; 
        m_road[*(it+1)].s_index = tmpKeyPoint2.s_index; 
        m_road[*(it+1)].s_keySet.insert(tmpKeyPoint1);
        
        tmpKeyPoint1 = tmpKeyPoint2;                                                                                //offset created before for loop so we can do this
        tmpKeyPoint2.s_ways.clear();                                                                                //cleanup
    };
    
    keyIds.clear();                                                                                                 //cleanup
    tmpKeyPoint1.s_ways.clear();                                                                                    //cleanup
};

//implementation of Dijkstra's algorithm
void RoadMap::FindShortestRoute(long src, long dest)
{
    std::queue<long> ids;
    bool isVisited[m_roadPoints.size()] = {false};                                                                  //none is yet visited
    long curr = 0;
    
    m_road[src].s_distance = 0.0;                                                                                   //distance of source keyPoint is always 0
    
    ids.push(m_road[src].s_id);                                                                                     //push id of source keyPoint to queue
    
    while(!ids.empty())                                                                                             //exits the loop when everything was checked and done
    {
        curr = ids.front();                                                                                         //take an element from queue than pop it out
        
        ids.pop();
        
        isVisited[m_road[curr].s_index] = 1;                                                                        //isVisited = 1 at index that is current keyPoint's index
        
        for(auto currKey : m_road[curr].s_keySet)                                                                   //check adjacent KeyPoints of the current one
        {
            if(!isVisited[currKey.s_index])                                                                         //if it was not visited, set it to visited and enqueue it's id
            {
                ids.push(currKey.s_id);
                isVisited[currKey.s_index] = true;
            };

            double tmpDist = m_road[curr].s_distance + currKey.s_weight;

            if(tmpDist < m_road[currKey.s_id].s_distance)                                                           //update distance to smaller if it can be done
            {
                m_road[currKey.s_id].s_distance = tmpDist;
                m_road[currKey.s_id].s_lastKey  = curr;
            };
        };
    };
    
    PrintRoute(src, dest);                                                                                          //print the solution
};

//print the way you need to go for the wanted road
void RoadMap::PrintRoute(long src, long dest)
{
    long curr = dest; // go from back to the start (reverse)
    std::vector<long> road;
    
    road.emplace_back(curr);                                                                                        //emplace the starting (dest) id to vector
    
    while(curr != src)                                                                                              //until it reaches the begining, push to vector and go backwards (towards src)
    {
        road.emplace_back(m_road[curr].s_lastKey);
        
        curr = m_road[curr].s_lastKey;
    };
    
    std::cout << "Road from " << src << " to " << dest << "\n\n";
    
    for(std::vector<long>::reverse_iterator it = road.rbegin(); it != road.rend(); it++)                            //print the solution
        std::cout << *it << " ";

    std::cout << "\n";

    road.clear(); // cleanup
}
