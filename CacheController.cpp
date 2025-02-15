#include <bits/stdc++.h>
// #include <unistd.h>
#define underline "\033[4m"
#define closeunderline "\033[24m"
#define ULL unsigned long long
using namespace std;

#define Address_Size 40
#define Cache_Size 32*1024    //in Bytes
#define Block_Size 64
#define Ways 8
#define Block_Offset_Size log(Block_Size)
#define no_Blocks Cache_Size/Block_Size
#define no_Sets no_Blocks/Ways
#define Index_Size log(no_Sets)
#define Tag_Size Address_Size-Index_Size-Block_Offset_Size

class CacheController
{
    public:
    enum states{I,MP,V};
    enum CPU_action{Read,Write};
    class Cache_Block
    {
        public:
        states State;
        ULL Tag;
        //Data is Assumed to be There
        Cache_Block()
        {
            State=I;
            Tag=0;
        }
    };
    class Cache_Set
    {
        public:
        vector<Cache_Block> Block;
        Cache_Set()
        {
            Block=vector<Cache_Block> (Ways);
        }
    };
    class CPUCache
    {
        public:
        vector<Cache_Set> Set;
        CPUCache()
        {
            Set=vector<Cache_Set> (no_Sets);
        }
    };
    CPUCache Cache;
    class Stage1
    {
        public:
        bool valid;
        int request_No;
        Stage1()
        {
            valid=true;
            request_No=0;
        }
    };
    class Stage2
    {
        public:
        bool valid;
        CPU_action request;
        ULL Address;
        Stage2()
        {
            valid=false;
            request=Read;
            Address=0;
        }
    };
    class Stage3
    {
        public:
        bool valid;
        CPU_action operation;
        ULL Tag,Index;
        //Block Offset Assumed
        Stage3()
        {
            valid=false;
            operation=Read;
            Tag=Index=0;
        }
    };
    class Stage4
    {
        public:
        bool valid;
        CPU_action operation;
        bool hit;
        int match;
        ULL Tag,Index;
        //Block Offset Assumed
        Stage4()
        {
            valid=false;
            hit=false;
            match=-1;
            Tag=Index=0;
        }
    };
    Stage1 RC;
    Stage2 RFRD;
    Stage3 RDEX;
    Stage4 EXCR;
    ULL hit,miss;
    unsigned long long CPU_Request_No;
    CacheController()
    {
        CPU_Request_No=0;
        hit=miss=0;
    }
    ULL RandAddress()
    {
        ULL min_val=0;
        ULL max_val=((ULL)1<<(ULL)(Address_Size-Block_Offset_Size))-1;
        ULL mean=(max_val-min_val)/2;
        ULL stddev=1<<6;
        static random_device rd;         
        static mt19937 gen(rd());        
        normal_distribution<> dist(mean, stddev); 
        ULL random_value = round(dist(gen));  
        random_value = max(min_val, min(max_val, random_value)); 
        return random_value;
    }
    CPU_action RandAction()
    {
        return (rand()%20>5)?Read:Write;
    }
    pair<CPU_action,ULL> RandRequest()
    {
        ULL address=RandAddress();
        CPU_action action=RandAction();
        return {action,address};
    }
    void Fetch()
    {
        if(RC.request_No<0||RC.request_No>=CPU_Request_No)
        {
            RC.valid=false;
        }
        if(!RC.valid)
        {
            RFRD.valid=false;
            return;
        }
        pair<CPU_action,ULL> CPU_Request=RandRequest();
        RFRD.request=CPU_Request.first;
        RFRD.Address=CPU_Request.second;
        RFRD.valid=true;
        RC.request_No+=1;
    }
    void Decode()
    {
        if(!RFRD.valid)
        {
            RDEX.valid=false;
            return;
        }
        RDEX.operation=RFRD.request;
        RDEX.Tag=RFRD.Address/(1<<(ULL)Index_Size);
        RDEX.Index=RFRD.Address%(1<<(ULL)Index_Size);
        RDEX.valid=true;
    }
    void Execute()
    {
        if(!RDEX.valid)
        {
            EXCR.valid=false;
            return;
        }
        EXCR.hit=false;
        EXCR.match=-1;
        for(int way_No=0;way_No<Ways;way_No++)
        {
            if(EXCR.match==-1&&Cache.Set[RDEX.Index].Block[way_No].State==I)
                EXCR.match=way_No;
            else if(Cache.Set[RDEX.Index].Block[way_No].State==V&&Cache.Set[RDEX.Index].Block[way_No].Tag==RDEX.Tag)
            {
                EXCR.hit=true;
                EXCR.match=way_No;
                break;
            }
        }
        EXCR.Tag=RDEX.Tag;
        EXCR.Index=RDEX.Index;
        EXCR.operation=RDEX.operation;
        EXCR.valid=true;
    }
    void CommitRetire()
    {
        if(!EXCR.valid)
            return;
        if(EXCR.hit)
        {
            hit+=1;       
        }
        else
        {
            miss+=1;
            if(EXCR.operation==Read)
            {
                if(EXCR.match!=-1)
                {
                    Cache.Set[EXCR.Index].Block[EXCR.match].State=MP;
                    Cache.Set[EXCR.Index].Block[EXCR.match].Tag=EXCR.Tag;
                    Cache.Set[EXCR.Index].Block[EXCR.match].State=V;
                }
                else
                {
                    int victim=rand()%Ways;
                    Cache.Set[EXCR.Index].Block[victim].State=I;
                    Cache.Set[EXCR.Index].Block[victim].State=MP;
                    Cache.Set[EXCR.Index].Block[victim].Tag=EXCR.Tag;
                    Cache.Set[EXCR.Index].Block[victim].State=V;
                }
            }
        }
    }
    double main(ULL n)
    {
        CPU_Request_No=n;
        // time_t start;
        // time(&start);
        // time_t end;
        bool validity=RC.valid||RFRD.valid||RDEX.valid||EXCR.valid;
        while(validity)
        {
            // cout<<RC.valid<<RFRD.valid<<RDEX.valid<<EXCR.valid<<endl;
            CommitRetire();
            Execute();
            Decode();
            Fetch();
            validity=RC.valid||RFRD.valid||RDEX.valid||EXCR.valid;
            // time(&end);
        }
        cout<<"Hits: "<<hit<<"\tMiss: "<<miss;
        double hit_rate=hit*100.0/(hit+miss);
        cout<<"\tHit Rate: "<<hit_rate<<endl;
        return hit_rate;
    }
};
void Graph(vector<vector<char>> graph)
{
    cout<<endl<<"Graph in a logarithmic scale:"<<endl;
    for(int x=graph.size()-1;x>=0;x--)
    {
        cout<<x;
        for(int y=0;y<graph[0].size();y++)
        {
            cout<<" "<<graph[x][y];
        }
        cout<<endl;
    }
    cout<<"e";
    for(int y=0;y<graph[0].size();y++)
        cout<<"--";
    cout<<"\n%";
    for(int y=0;y<graph[0].size();y++)
    {
        if(y%5)
            cout<<"  ";
        else
            cout<<" "<<y*10/(graph[0].size()-1);
    }
    cout<<endl;
}
int main()
{
    ULL Limit=(ULL)1e7;
    CacheController* Simulate;
    int total_sims=0;
    double avg_hit_rate=0;
    double hit_rate;
    ULL n=1;
    bool toggle=false;
    double togg_avg;
    vector<vector<char>> graph ((int)log10(Limit)+1,vector<char> (25+1,' '));
    while(n<=Limit)
    {
        if(!toggle)
            cout<<endl<<underline<<n<<" Requests:"<<closeunderline<<endl;
        Simulate=new CacheController();
        hit_rate=Simulate->main(n);
        delete(Simulate);
        togg_avg+=hit_rate;
        avg_hit_rate+=hit_rate;
        if(toggle)
        {
            togg_avg/=2;
            cout<<"Average: "<<togg_avg<<endl;
            togg_avg*=graph[0].size()/100.0;
            graph[(int)log10(n)][(int)togg_avg]='*';
            togg_avg=0;
            n*=10;
        }
        toggle=!toggle;
        total_sims+=1;
    }
    avg_hit_rate/=total_sims;
    cout<<endl<<"Average Hit Rate: "<<avg_hit_rate<<endl;
    Graph(graph);
    return 0;
}