#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iterator>
#include <map>
#include <vector>
#include <ctime>
#include "Ngram.h"
#define ngram_order 2
using namespace std;
Vocab voc;
Ngram lm( voc, ngram_order );

vector<string> split(string &str) 
{
    string buf;
    vector<std::string> tokens;
    istringstream iss(str);
    int i =0;
    while(iss >> buf)
    {
        tokens.push_back(buf);
    }
    return tokens;

}

void Print(vector<string>& v)
{
    for(int i=0 ; i<v.size(); i++)
    {
        vector<string> temp;
        temp = split(v[i]);
        for(int j=0  ; j<temp.size(); j++)
        {
            cout << temp[j] << " ";
        }
        cout << endl;
    }
}

map<string, vector<string> > ReadMappingFile(char* filename)
{
    string line; 
    vector<string> tokens;
    ifstream mapping_file(filename, ios::binary);
    char SingleWord[2], temp[2];
    map<string, vector<string> > Words;
    while(getline(mapping_file, line))
    {
        tokens = split(line);
        Words.insert(make_pair(tokens[0], tokens));
    }
    return Words;
}

vector<string> ReadInputFile(char* filename)
{
    string line, token;
    ifstream mapping_file(filename, ios::binary);
    char SingleWord[2], temp[2];
    vector<string> lines;
    while(getline(mapping_file, line))
    {
        lines.push_back(line);
    }
    return lines;

}

double getBigramProb(const char *w1, const char *w2)
{
    VocabIndex wid1 = voc.getIndex(w1);
    VocabIndex wid2 = voc.getIndex(w2);

    if(wid1 == Vocab_None)  //OOV
        wid1 = voc.getIndex(Vocab_Unknown);
    if(wid2 == Vocab_None)  //OOV
        wid2 = voc.getIndex(Vocab_Unknown);

    VocabIndex context[] = { wid1, Vocab_None };
    return lm.wordProb( wid2, context);
}
void Veterbi(string sentence, map<string, vector<string> > mapping_list)
{
    vector<string> Words = split(sentence);
    vector <string> tokens;
    double prob = -1000;
    double TransProb = -1000;
    double max_prop = -1000;
    int maxWordIndex = -1;
    prob = getBigramProb("", Words[0].c_str());
    tokens = mapping_list[ Words[0] ];
    
    for(int j=1 ; j<tokens.size() ; j++)
    {
        TransProb = getBigramProb(tokens[j].c_str(), Words[1].c_str());
        if(TransProb + prob > max_prop)
        {
            max_prop = TransProb + prob;
            Words[0] = tokens[j];
        }
    }

    for(int i=1 ; i<Words.size(); i++)
    {
        tokens = mapping_list[ Words[i] ];
        TransProb = -1000;
        max_prop = -1000;
        maxWordIndex = -1;
        //j starts from 1 due to struture of the map  key: (key value)
        for(int j=1 ; j<tokens.size() ; j++)
        {
            TransProb = getBigramProb(Words[i-1].c_str(), tokens[j].c_str());

            if(TransProb + prob > max_prop)
            {
                max_prop = TransProb + prob;
                Words[i] = tokens[j];
            }
        }
        
                    prob = max_prop;
        prob += getBigramProb("", Words[i].c_str());
    }
    cout << "<s> ";
    for(int i=0 ; i<Words.size() ; i++)
        cout << Words[i] << " ";
    cout << " </s>" << endl;
}

int main(int argc, char *argv[])
{
    char* lm_filename = argv[2];
    File lmFile(lm_filename, "r");
    lm.read(lmFile);
    lmFile.close();
    vector<string> lines = ReadInputFile(argv[1]);
    map<string, vector<string> > mapping_list = ReadMappingFile("./ZhuYin-Big5.map");
    for(int i=0 ; i<lines.size() ; i++)
        Veterbi(lines[i], mapping_list);

    return 0;
}
