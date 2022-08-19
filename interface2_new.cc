#include <chrono>
#include <thread>
#include <iostream>
#include <iterator>
#include <unistd.h>
#include <fstream>
#include <chrono>
#include <map>
#include <cstdio>
#include <fcntl.h>
#include <sstream>
#include <vector>
#include <algorithm>
#include <condition_variable>
#include <mutex>
#include <shared_mutex>
#include <memory>
#include <string>
#include <cctype>

using namespace std;

shared_timed_mutex _protect;
map<string, string> kv;
map<string, string> kv1;
//mutex(mu);                    //Global variable or place within class
condition_variable condition; //A signal that can be used to communicate between functions
map<string, string>::iterator itr1;
vector<string> file_name;
std::vector<std::mutex *> my_mutexes(26);
std::vector<shared_timed_mutex *> my_mutexes1(26);

//Initialize mutexes

vector<string> keyy;
map<string, string>::iterator itr;
map<string, string>::iterator it;

int change = 0;

class Cache_base
{
public:
    virtual string getValue(string key) { return NULL; };
    virtual void set(string key, string value){};
    virtual void deletekey(string key){};
};
Cache_base *cache;
class LRUCache : public Cache_base
{
public:
    class Node
    {
    public:
        string key, value;
        Node *left;

        Node *right;
        Node(string k, string v)
        {
            key = k;
            value = v;
        }
    };

    LRUCache(int sz)
    {
        size = sz;
        head->right = tail;
        tail->left = head;
        tail->right = NULL;
    }
    Node *head = new Node("0", "0");
    Node *tail = new Node("0", "0");
    void addNode(string key, string value)
    {
        Node *temp = new Node(key, value);
        temp->right = head->right;
        temp->left = head;
        (head->right)->left = temp;
        head->right = temp;
    }
    void deleteNode(Node *node)
    {
        Node *prev = node->left;
        Node *next = node->right;
        prev->right = next;
        next->left = prev;
        delete node;
    }
    map<string, Node *> map;
    int size;
    string getValue(string key)
    {
        string value;
        if (map.find(key) != map.end())
        {
            //Node* temp= cache;
            //cout << "here 111 " << key << " \n";
            Node *val = map[key];
            map.erase(key);
            value = val->value;
            deleteNode(val);
            addNode(key, value);
            map[key] = head->right;
            return value;
        }
        else
        {
            //should refer from file here and add value
            //value=readfromfile(key);
            //set(key,value);
            value = "-1";
            return value;
        }
    }
    void deleteValues(string key)
    {
        if (map.find(key) != map.end())
        {
            Node *temp = map[key];
            map.erase(key);
            deleteNode(temp);
        }
        //call delete from file routine from here
    }
    void set(string key, string value)
    {
        if (map.find(key) != map.end())
        {
            Node *temp = map[key];
            map.erase(key);
            deleteNode(temp);
        }
        if (map.size() == size)
        {
            //cout << key <<" here" << "\n";
            map.erase(tail->left->key);
            deleteNode(tail->left);
        }

        addNode(key, value);
        map[key] = head->right;
    }
    void getall()
    {
        Node *temp = head->right;
        while (temp->right)
        {
            temp = temp->right;
        }
    }
};

class LFUCache : public Cache_base
{
public:
    class Node
    {
    public:
        string key, value;
        unsigned int freq;
        Node *left;
        Node *right;
        Node(string k, string v)
        {
            key = k;
            value = v;
            freq = 1;
        }
    };
    class DLL
    {
    public:
        Node *head, *tail;
        int size;
        DLL()
        {
            head = new Node("0", "0");
            tail = new Node("0", "0");
            head->right = tail;
            tail->left = head;
            tail->right = NULL;
            size = 0;
        }
        void addNode(Node *temp)
        {
            if (size < 0)
                return;
            // Node *temp=new Node(key,value);
            temp->right = head->right;
            temp->left = head;
            (head->right)->left = temp;
            head->right = temp;
            size++;
        }
        void deleteNode(Node *node)
        {
            Node *prev = node->left;
            Node *next = node->right;
            prev->right = next;
            next->left = prev;
            size--;
        }
    };
    int size, minfreq;
    map<string, Node *> nodes;
    map<int, DLL *> freqs;

    LFUCache(int sz)
    {
        size = sz;
        minfreq = 0;
    }
    void update(Node *temp)
    {
        DLL *olddll = freqs[temp->freq];
        olddll->deleteNode(temp);
        if (temp->freq == minfreq && olddll->size == 0)
            minfreq++;
        if (olddll->size == 0)
        {
            freqs.erase(temp->freq);
            delete olddll;
        }
        temp->freq++;
        DLL *dll;
        if (freqs.find(temp->freq) != freqs.end())
        {
            dll = freqs[temp->freq];
        }
        else
        {
            dll = new DLL();
        }
        dll->addNode(temp);
        freqs[temp->freq] = dll;
    }
    string getValue(string key)
    {
        if (nodes.find(key) != nodes.end())
        {
            Node *temp = nodes[key];
            update(temp);
            return temp->value;
        }
        else
        {
            // return value from file;
            //insert into cache
            return "-1";
        }
    }

    void set(string key, string value)
    {
        if (nodes.find(key) != nodes.end())
        {
            Node *temp = nodes[key];
            temp->value = value;
            update(temp);
        }
        else
        {
            Node *node = new Node(key, value);
            if (nodes.size() == size)
            {
                //max size delete small node
                DLL *mindll = freqs[minfreq];
                nodes.erase(mindll->tail->left->key);
                mindll->deleteNode(mindll->tail->left);
            }
            minfreq = 1;
            nodes[key] = node;
            DLL *mindll;
            if (freqs.find(minfreq) != freqs.end())
            {
                mindll = freqs[minfreq];
            }
            else
            {
                mindll = new DLL();
            }
            mindll->addNode(node);
            freqs[minfreq] = mindll;
        }
    }
    int getMaxNext(int minfreq)
    {
        int min = -1;
        for (map<int, DLL *>::iterator it = freqs.begin(); it != freqs.end(); ++it)
        {
            if (it->first > min && freqs[it->first]->size != 0)
            {
                min = it->first;
            }
        }
        return min;
    }
    void deletekey(string key)
    {
        if (nodes.find(key) == nodes.end())
        {
            return;
        }
        else
        {
            Node *temp = nodes[key];
            nodes.erase(key);
            DLL *dll = freqs[temp->freq];
            dll->deleteNode(temp);
            if (dll->size == 0 && minfreq == temp->freq)
            {
                freqs.erase(temp->freq);
                minfreq = getMaxNext(minfreq);
            }
            if (dll->size == 0)
                freqs.erase(temp->freq);
            delete temp;
        }
    }
};
void config_cache(int cachetype, int size)
{

    if (cachetype == 1)
    {
        cache = new LFUCache(size);
    }
    else
    {
        cache = new LRUCache(size);
    }
}
int put(string key, string value)
{

    if (key.empty() || value.empty())
    {
        return 4; //key or value is empty
    }
    else
    {
        if (key.size() > 256 && value.size() > 256)
        {
            return 3; //key or value string byte is more than 256 byte
        }

        else
        {

            

            int index1 = 0;
            index1 = tolower(key.at(0)) - 'a';
            
            if (find(keyy.begin(), keyy.end(), key) != keyy.end())
            {
                
                string res = cache->getValue(key);
                if (res == "-1")
                {
                    unique_lock<shared_timed_mutex> w(*my_mutexes1[index1]);

                    std::ifstream inFile;
                    inFile.open(file_name[index1]); //open the input file

                    std::stringstream strStream;
                    strStream << inFile.rdbuf();       //read the file
                    std::string str = strStream.str(); //str holds the content of the file
                    
                    int index = str.find(key);
                    int count = index + key.length();
                    int found = str.find('\n', count);
                    string r = str.substr(count + 1, found - count - 1);
                    str.replace(count + 1,found - count - 1, value);
                    ofstream fp(file_name[index1]);
                    fp << str;
                    fp.flush();
                    cache->set(key, r);
                }

                else
                {
                    cache->set(key, value);

                    int index1 = 0;
                    index1 = tolower(key.at(0)) - 'a';
                    unique_lock<shared_timed_mutex> w(*my_mutexes1[index1]);

                    std::ifstream inFile;
                    inFile.open(file_name[index1]); //open the input file

                    std::stringstream strStream;
                    strStream << inFile.rdbuf();       //read the file
                    std::string str = strStream.str(); //str holds the content of the file
                    int index = str.find(key);
                    int count = index + key.length();
                    int found = str.find('\n', count);
                    
                        string r = str.substr(count + 1, found - count - 1);
                        str.replace(count + 1,found - count - 1, value);
                        ofstream fp(file_name[index1], std::ios_base::app);
                        fp << key << "=" << value << endl;
                        fp.flush();
                        fp.close();

                        
                }
            }
            else
            {

                keyy.push_back(key);

                int index1 = 0;
                index1 = tolower(key.at(0)) - 'a';
                unique_lock<shared_timed_mutex> w(*my_mutexes1[index1]);

                ofstream fp(file_name[index1], std::ios_base::app);
                fp << key << "=" << value << endl;
                fp.flush();
                fp.close();
            }
        }
    }
    return 1;
}

int delete1(string key)
{
    if (key.empty())
    {
        return 4; //key or value is empty
    }
    else
    {

        if (key.size() > 256)
        {
            return 3; //key or value string byte is more than 256 byte
        }

        else
        {
            //unique_lock<shared_timed_mutex> w(_protect);
            //Do Stuff

            remove(keyy.begin(), keyy.end(), key);
            string res = cache->getValue(key);
            if (res == "-1")
            {
                int index1 = 0;
                index1 = tolower(key.at(0)) - 'a';
                unique_lock<shared_timed_mutex> w(*my_mutexes1[index1]);

                std::ifstream inFile;
                inFile.open(file_name[index1]); //open the input file
                if (inFile.is_open())
                {
                    std::stringstream strStream;
                    strStream << inFile.rdbuf();       //read the file
                    std::string str = strStream.str(); //str holds the content of the file
                    
                    int index = str.find(key);
                    int count = index + key.length();
                    int found = str.find('\n', count);
                    if (index == -1)
                    {
                        return 2; //key not found
                    }
                    string r = str.substr(count + 1, found - count - 1);
                    str.replace(index, key.length() + 2 + found - count - 1, "");
                    ofstream fp(file_name[index1]);
                    fp << str;
                    fp.flush();
                    fp.close();
                }
                else
                {
                    //cout << "File is not there\n";
                }
            }
            else
            {
                cache->deletekey(key);
                int index1 = 0;
                index1 = tolower(key.at(0)) - 'a';
                unique_lock<shared_timed_mutex> w(*my_mutexes1[index1]);

                std::ifstream inFile;
                inFile.open(file_name[index1]); //open the input file

                std::stringstream strStream;
                strStream << inFile.rdbuf();       //read the file
                std::string str = strStream.str(); //str holds the content of the file
                
                int index = str.find(key);
                int count = index + key.length();
                int found = str.find('\n', count);
                string r = str.substr(count + 1, found - count - 1);
                str.replace(index, key.length() + 2 + found - count - 1, "");
                ofstream fp(file_name[index1]);
                fp << str;
                fp.flush();
                fp.close();
            }
        }
        return 1;
    }
}
string get12(string key)
{
    if (key.empty())
    {
        return "4"; //key or value is empty
    }
    else
    {

        if (key.size() > 256)
        {
            return "3"; //key or value string byte is more than 256 byte
        }

        else
        {

            string res = cache->getValue(key);
            if (res == "-1")
            {
                int index1 = 0;
                index1 = tolower(key.at(0)) - 'a';
                shared_lock<shared_timed_mutex> r(*my_mutexes1[index1]); //std::shared_lock will shared_lock() the mutex.


                std::ifstream inFile;
                inFile.open(file_name[index1]); //open the input file

                std::stringstream strStream;
                strStream << inFile.rdbuf();       //read the file
                std::string str = strStream.str(); //str holds the content of the file
                
                int index = str.find(key);
                int count = index + key.length();
                int found = str.find('\n', count);
                if (index == -1)
                {
                    return "2"; //key not found
                }
                else
                {
                    string r = str.substr(count + 1, found - count - 1);
                    cache->set(key, r);
                    return r;
                }
            }
            else
            {
                return res;
            }
        }
        return "1";
    }
}
void run()
{
    for (int i = 0; i < 26; ++i)
    {
        my_mutexes[i] = new std::mutex();
    }
    for (int i = 0; i < 26; ++i)
    {
        my_mutexes1[i] = new std::shared_timed_mutex();
    }

    // Constructs the new thread and runs it. Does not block execution.
    string res;

    for (int i = 0; i < 26; i++)
    {
        string base = ".txt";
        string dir_path = "./data/";
        base.insert(0, 1, char('a' + i));
        file_name.push_back(dir_path + base.c_str());
    }
    file_name.push_back("./data/extra.txt");
}
