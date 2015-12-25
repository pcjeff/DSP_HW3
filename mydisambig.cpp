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
typedef struct VNode{
    string word;
    double prob;
    int prev; //prev connected node index
} VNode;
VNode **Vtable=NULL;

vector<string> split(string &str) 
{
    string buf, _;
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
void PrintVtableSeq(int sentence_len)
{
    string strbuf = " </s>\n";
    double prob=-1000, max_prop=-1000;
    int maxProbindex = -1, PrevIndex = -10000;

    for(int i=0 ; i<10000 ; i++)
    {
        if (Vtable[sentence_len-1][i].prob > max_prop)
        {
            max_prop = Vtable[sentence_len-1][i].prob;
            PrevIndex = Vtable[sentence_len-1][i].prev;
            maxProbindex = i;
        }
    }
    strbuf = " " + Vtable[sentence_len-1][maxProbindex].word + strbuf;
    for(int i=sentence_len-1 ; i>0 ; i--)
    {
        strbuf = " " + Vtable[i-1][PrevIndex].word + strbuf;
        PrevIndex = Vtable[i-1][PrevIndex].prev;
    }
    strbuf = "<s> " + strbuf;
    cout << strbuf << endl;
}
void Veterbi(string sentence, map<string, vector<string> > mapping_list)
{
    vector<string> Words = split(sentence);
    vector <string> tokens;
    string strbuf = "";
    double prob = -1000;
    double TransProb = -1000;
    double max_prop = -1000;
    int prevTokenMappingListLen = -1, curTokenMappingListLen= -1;
    prob = getBigramProb("", Words[0].c_str());
    tokens = mapping_list[ Words[0] ];
    //Viteribi tabe
    Vtable = (VNode**)malloc(sizeof(VNode*)*Words.size());
    for(int i=0 ; i<Words.size() ; i++)
        Vtable[i] = (VNode*)malloc(sizeof(VNode)*10000);
    for(int i=0 ; i<Words.size() ; i++)
        for(int j=0 ; j<10000 ; j++)
        {
            Vtable[i][j].word = "<unk>";
            Vtable[i][j].prob = -10000;
            Vtable[i][j].prev = -10000;
        }

    prevTokenMappingListLen = tokens.size();
    for(int j=0 ; j<tokens.size() ; j++)
    {
        TransProb = getBigramProb("<s>", tokens[j].c_str());
        if(TransProb > max_prop)
        {
            max_prop = TransProb;
            Vtable[0][j].prob = max_prop;
            Vtable[0][j].word = tokens[j];
        }
    }

    for(int i=1 ; i<Words.size(); i++)
    {
        tokens = mapping_list[ Words[i] ];
        curTokenMappingListLen = tokens.size();
        for(int k=0 ; k<curTokenMappingListLen ; k++)
        {
            TransProb = -1000;
            max_prop = -1000;
            for(int j=0 ; j<prevTokenMappingListLen ; j++)
            {
                TransProb = getBigramProb(Vtable[i-1][j].word.c_str(), tokens[k].c_str());

                if(TransProb + Vtable[i-1][j].prob > max_prop)
                {
                    max_prop = TransProb + Vtable[i-1][j].prob;
                    Vtable[i][k].word = tokens[k];
                    Vtable[i][k].prob = max_prop + getBigramProb("", Words[i].c_str());
                    Vtable[i][k].prev = j;
                }
            }
        }
        prevTokenMappingListLen = curTokenMappingListLen;
    }
    PrintVtableSeq(Words.size());
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
