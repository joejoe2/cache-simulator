#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdio.h>
#include <list>
#include<math.h>
#include<exception>
using namespace std;

class CacheBlocks{
	public:
	int tag;
	bool valid;
	int usedTimes;
	CacheBlocks(int tag,bool valid){
	  this->tag=tag;
	  this->valid=valid;
	}
	
};

int main(int argc, char *argv[]){
	int lineSize=32;//(bits)
	int cacheSize=0;//(kbyte)
	int blockSize=0;//(byte)
	int entryNum=0;
	int occupy=0;
	int offsetSize=0;//(bits)
	int indexSize=0;
	int tagSize=0;
	int associativity=0;// associativity
	int policy=0;//policy
	int way=0;
	
	ifstream file(argv[1]/*"trace1.txt"*/);
	ofstream fileOut(argv[2]/*"trace1.out"*/,ios::trunc);
    string line;
    
    getline(file, line);
    cacheSize=atoi(line.c_str());
    
    getline(file, line);
    blockSize=atoi(line.c_str());
    entryNum=(cacheSize*1024)/blockSize;
    offsetSize=log(blockSize)/log(2);
    
    
	getline(file, line);
    associativity=atoi(line.c_str());
    if(associativity==0){//direct map 1-way
    	indexSize=log(entryNum)/log(2);
        tagSize=32-offsetSize-indexSize;
	}else if(associativity==1){//4-way
		way=4;
		indexSize=log(entryNum/way)/log(2);
        tagSize=32-offsetSize-indexSize;
	}else if(associativity==2){//fully associativity  entryNum-way
		way=entryNum;
		indexSize=log(entryNum/way)/log(2);
        tagSize=32-offsetSize-indexSize;
	}
    
	getline(file, line);
    policy=atoi(line.c_str());
    
    //cout<<"policy: "<<policy<<" way: "<<way<<" cache size:"<<cacheSize<<" blockSize:"<<blockSize<<" entryNum:"<<entryNum<<" offsetSize:"<<offsetSize<<" indexSize:"<<indexSize<<" tagSize:"<<tagSize<<endl;
    CacheBlocks *cache[entryNum];
	for(int i=0;i<entryNum;++i){
		cache[i]=new CacheBlocks(0,false);
	}
	
	//cout<<(int)pow(2,indexSize)<<endl;
	list<int> lru[(int)pow(2,indexSize)];
	list<int> fifo[(int)pow(2,indexSize)];
	
    while (getline(file, line)) {	
        if(associativity==0){//direct map    replace the mapped block directly!
		unsigned int x,index,tag;
		int victimTag=-1;
        bool hit=false;
        sscanf(line.c_str(),"%x", &x);
        tag=x>>(indexSize+offsetSize);
        index=(x<<tagSize)>>(tagSize+offsetSize);
        if(cache[index]->valid&&cache[index]->tag==tag){
		   hit=true;
		}else{
			if(cache[index]->valid){
				victimTag=cache[index]->tag;
			}else{
				cache[index]->valid=true;
				++occupy;
			}
			cache[index]->tag=tag;
			hit=false;
		}
        //cout<<x<<" tag:"<<tag<<" index:"<<index<<" hit:"<<hit<<" victimTag:"<<victimTag<<endl;
        fileOut<<victimTag<<endl;
		}else if(associativity==1){//4-way
			unsigned int x,index,tag;
		    int victimTag=-1;
		    int free=-1;
            bool hit=false;
            sscanf(line.c_str(),"%x", &x);
            tag=x>>(indexSize+offsetSize);
            index=(x<<tagSize)>>(tagSize+offsetSize);//index=set
            int set=index;
            int small=2147483647,mytarget=0;
            //
            //cout<<set<<endl;
            //
            for(int i=set*way;i<set*way+way;++i){
            	if(cache[i]->valid&&cache[i]->tag==tag){
		            hit=true;
		            lru[set].remove(i);
		            lru[set].push_front(i);
		            cache[i]->usedTimes++;
					break;
		        }else if(!cache[i]->valid){
		        	free=i;
				}else if(cache[i]->valid&&cache[i]->usedTimes<small){
		        	small=cache[i]->usedTimes;
		        	mytarget=i;
				}
			}
			if(!hit){
				if(free!=-1){//has free
					cache[free]->valid=true;
					cache[free]->tag=tag;
					lru[set].push_front(free);
					fifo[set].push_back(free);
					cache[free]->usedTimes++;
				}else{//need replace
				    int des=0;
					if(policy==0){//fifo
					des=fifo[set].front();
					fifo[set].pop_front();
					victimTag=cache[des]->tag;
					fifo[set].push_back(des);
					}
					else if(policy==1){//lru
					des=lru[set].back();
					lru[set].pop_back();
					victimTag=cache[des]->tag;
					lru[set].push_front(des);
				    }
					else{//my policy
					
					des=lru[set].back();
					lru[set].pop_back();
					victimTag=cache[des]->tag;
					lru[set].push_front(des);
					
					//des=fifo[set].front();
					//fifo[set].pop_front();
					//victimTag=cache[des]->tag;
					//fifo[set].push_back(des);
					/*des=mytarget;
					cache[des]->usedTimes=0;
					victimTag=cache[des]->tag;*/
					}
				    cache[des]->tag=tag;
				    cache[des]->usedTimes++;
				    //if(victimTag!=-1)cout<<victimTag<<endl;
				    
				}
			}
			
			fileOut<<victimTag<<endl;
		}else if(associativity==2){//fully associativity 
			unsigned int x,index,tag;
		    int victimTag=-1;
		    int free=-1;
            bool hit=false;
            sscanf(line.c_str(),"%x", &x);
            tag=x>>(indexSize+offsetSize);
            index=(x<<tagSize)>>(tagSize+offsetSize);
            int small=2147483647,mytarget=0;
            //
            for(int i=0;i<entryNum;++i){
            	if(cache[i]->valid&&cache[i]->tag==tag){
		            hit=true;
		            lru[0].remove(i);
		            lru[0].push_front(i);
		            cache[i]->usedTimes++;
					break;
		        }else if(!cache[i]->valid){
		        	free=i;
				}else if(cache[i]->valid&&cache[i]->usedTimes<small){
		        	small=cache[i]->usedTimes;
		        	mytarget=i;
				}
			}
			if(!hit){//miss need place a block
				if(free!=-1){//has free
					cache[free]->valid=true;
					cache[free]->tag=tag;
					lru[0].push_front(free);
					fifo[0].push_back(free);
					cache[free]->usedTimes++;
				}else{//need replace
				    int des=0;
					if(policy==0){//fifo
					des=fifo[0].front();
					fifo[0].pop_front();
					victimTag=cache[des]->tag;
					fifo[0].push_back(des);
					}
					else if(policy==1){//lru
					des=lru[0].back();
					lru[0].pop_back();
					victimTag=cache[des]->tag;
					lru[0].push_front(des);
				    }
					else{//my policy
					
					des=lru[0].back();
					lru[0].pop_back();
					victimTag=cache[des]->tag;
					lru[0].push_front(des);
				    //des=mytarget;
				    //cache[des]->usedTimes=0;
					//victimTag=cache[des]->tag;
				    }
				    cache[des]->tag=tag;
				    cache[des]->usedTimes++;
				    //if(victimTag!=-1)cout<<victimTag<<endl;
				}
			}
			
	    //cout<<x<<" tag:"<<tag<<" index:"<<index<<" hit:"<<hit<<" victimTag:"<<victimTag<<endl;
        fileOut<<victimTag<<endl;
		}
    }
    
    file.close();
    fileOut.close();
}

