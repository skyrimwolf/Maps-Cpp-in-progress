#include<stdio.h>
#include<math.h>
#include<climits>
#include<string>
#include<vector>
#include<fstream>
#include<unordered_map>
#include<unordered_set>
#include<set>
#include<queue>

#if 0                                                      //if 1 if you want to check speed
#include<chrono>

struct Timer
{
    std::string s_name;
    std::chrono::high_resolution_clock::time_point s_start, s_end;
    std::chrono::duration<float> s_duration;

    Timer(const char *name)
    {
        s_start = std::chrono::high_resolution_clock::now();
        s_name  = name;
    }

    ~Timer()
    {
        s_end      = std::chrono::high_resolution_clock::now();
        s_duration = s_end - s_start;

        printf("Timer for %s took %f ms.\n", s_name.c_str(), s_duration.count()*1000.0f);
    }
};
#endif

struct Way
{
    long s_wayId       = 0;
    bool s_oneWay      = false;
    std::string s_name = "NONAME";
};

struct KeyPoint
{
    long   s_index     = 0;
    long   s_id        = 0;
    double s_latitude  = 0.0;
    double s_longitude = 0.0;
    double s_weight    = LONG_MAX;                       //preparation for Dijkstra's algorithm
    std::vector<Way> s_ways;                             //KeyPoints can be on a crossway so it can be a part of multiple streets
};

struct Road
{
    long s_id         = 0;                               //id of a KeyPoint that has its set of connections
    long s_index      = 0;                               //index of a KeyPoint that has its set of connections
    long s_lastKey    = 0;                               //parent key - preparation for Dijkstra's algorithm
    double s_distance = (double)LONG_MAX;                //distance from the src id
    std::set<KeyPoint> s_keySet;                         //set of connections for a given KeyPoint with index s_index
};

std::unordered_map<std::string, int> g_nameTag           //quick choosing
{
    {"way"     , 1},
    {"id"      , 2},
    {"oneway"  , 3},
    {"name"    , 4},
    {"operator", 5}
};

bool operator <(const KeyPoint &x, const KeyPoint &y)    //a must: order of set is decided by the weight of a KeyPoint - preparation for Dijkstra's algorithm
{
    return x.s_weight < y.s_weight;
}

bool operator==(KeyPoint const &x, KeyPoint const &y)    //a must because of unordered_map/unordered_set
{
    return x.s_id == y.s_id;
}

template<>                                               //do not delete
struct std::hash<KeyPoint>                               //a must because of unordered_map/unordered_set
{
    size_t
    operator()(const KeyPoint & obj) const
    {
        return std::hash<int>()(obj.s_id);
    }
};
