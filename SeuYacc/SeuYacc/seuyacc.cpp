#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include <stack>
#include<queue>
#include<fstream>
#include<vector>
#include<map>
#include<string>
#include<set>
using namespace std;
#define BUFF 512
bool isused = true;
bool isepsilon = false;
struct production
{
	string leftp;
	string rightp[10];
	int numofrights;
	bool ifsubp;
	string subp;
};
vector<production> productions(100);
struct token {
	string name;
	string type;
};
queue<token> tokenqueue;
struct table2 {
	vector<string> tg;
	vector<int> next;
};
void printbuffer(vector<string> a)
{
	vector<string>::iterator i;

	for (i = a.begin(); i != a.end(); i++)
	{
		cout << (*i) << " ";
	}
}
int num = 0;
string newTemp()
{
	return "t" + to_string(++num);
}
int countquat = 0;
struct variable {
	string name;
	string type;
};
vector<variable> variableTable;
struct dhinfo
{
	bool h = 0;//活跃状态
	int d = -1;//待用信息
};
struct quat
{
	//2021 1.8
	int seq;
	string op;
	string op1;
	string op2;
	string res;
	dhinfo op1info;
	dhinfo op2info;
	dhinfo resinfo;
	int num;
	bool ifnewtemp;
};
vector<quat> quattable;//四元式向量
quat quatdeal(string subp)
{
	if (subp.find("new") != string::npos)
	{
		bool ifnewtemp = 1;
		char* a = (char*)subp.data();
		char* temp = strtok(a, ";");
		temp = strtok(NULL, ",");
		string temp2 = temp;
		temp2.substr(1, temp2.length() - 1);
		string op = temp2;
		temp = strtok(NULL, ",");
		string op1 = temp;
		temp = strtok(NULL, ",");
		string op2 = temp;
		temp = strtok(NULL, ";");
		string res = temp;
		int num;
		if (op1 == "@" && op2 == "@")
			num = 0;
		else if (op1 == "@" || op2 == "@")
			num = 1;
		else
			num = 2;
		quat qgf;
		qgf.op = op; qgf.op1 = op1; qgf.op2 = op2; qgf.res = res; qgf.num = num; qgf.ifnewtemp = ifnewtemp;
		return qgf;
	}
	else {
		bool ifnewtemp = 0;
		char* a = (char*)subp.data();
		char* temp = strtok(a, ",");
		string op = temp;
		temp = strtok(NULL, ",");
		string op1 = temp;
		temp = strtok(NULL, ",");
		string op2 = temp;
		temp = strtok(NULL, ";");
		string res = temp;
		int num;
		if (op1 == "@" && op2 == "@")
			num = 0;
		else if (op1 == "@" || op2 == "@")
			num = 1;
		else
			num = 2;
		quat qgf;
		qgf.op = op; qgf.op1 = op1; qgf.op2 = op2; qgf.res = res; qgf.num = num; qgf.ifnewtemp = ifnewtemp;
		return qgf;
	}
}
int offset = 0;
struct symbol
{
	string type;
	string name;
	int length;
	int offset;
	dhinfo info;
};
vector<symbol> symboltable;

//2021 1.8
stack<int> TC;
stack<int> FC;

//2021 1.9 基本块
struct BasicBlock
{
	set<string> outLiveVar;//出口活跃变量集
	int begin;//块首句四元式序号
	int end;//块尾句四元式序号
	vector<int> predecessors;//基本块前驱
	vector<int> successors;//基本块后继
	set<string> inLiveVar;//入口活跃变量集
	int seq;
	BasicBlock(int index = -1) {
		begin = index;
	}
};
map<int, BasicBlock> flowGragh;
set<int> firstnum;//基本块首指令
void divideBasicBlock(vector<quat> quattable)
{
	firstnum.insert(0);
	for (int i = 0; i < quattable.size(); i++)
	{
		if (quattable[i].op == "j" || quattable[i].op == "jz")
		{
			firstnum.insert(stoi(quattable[i].res));
			firstnum.insert(i + 1);
		}
		if (i == quattable.size() - 1)
		{
			firstnum.insert(i + 1);
		}
	}
}
void genFlowgragh(set<int> firstnum)
{
	BasicBlock temp;
	int i = 0;
	for (set<int>::iterator it = firstnum.begin(); it != firstnum.end(); )
	{
		temp.begin = (*it);
		if (++it != firstnum.end())
			temp.end = *(it)-1;
		else
		{
			temp.begin = *(--it);
			temp.end = temp.begin;
			temp.seq = i;
			i++;
			flowGragh.insert(pair<int, BasicBlock>(temp.begin, temp));
			break;
		}
		temp.seq = i;
		i++;
		flowGragh.insert(pair<int, BasicBlock>(temp.begin, temp));
	}
	map<int, BasicBlock>::iterator iter;
	map<int, BasicBlock>::iterator iter2;
	for (iter = flowGragh.begin(); iter != flowGragh.end();) {
		iter2 = iter;
		if (++iter2 != flowGragh.end())
		{
			//cout << (*iter).first << endl;
			if (quattable[(*iter).second.end].op == "jz")
			{
				(*iter).second.successors.push_back(stoi(quattable[(*iter).second.end].res));
				iter2 = iter;
				(*iter2).second.successors.push_back((*(++iter)).first);
			}
			else if (quattable[(*iter).second.end].op == "j")
			{
				(*iter).second.successors.push_back(stoi(quattable[(*iter).second.end].res));
				++iter;
			}
			else {
				iter2 = iter;
				(*iter2).second.successors.push_back((*(++iter)).first);

			}
		}
		else
			break;
	}//后继
	for (iter = flowGragh.begin(); iter != flowGragh.end();) {
		iter2 = iter;
		if (++iter2 != flowGragh.end())
		{
			for (int i = 0; i < (*iter).second.successors.size(); i++)
			{
				(*flowGragh.find((*iter).second.successors[i])).second.predecessors.push_back((*iter).second.begin);
			}
			iter++;
		}
		else
			break;
	}
}
void clearSymbol(vector<symbol>& symboltable)
{
	for (int i = 0; i < symboltable.size(); i++)
	{
		symboltable[i].info.d = -1;
		symboltable[i].info.h = 0;
	}
}
void fillBlockdhinfo(int begin, int end, set<string>& outLiveVar, set<string>& inLiveVar)
{
	clearSymbol(symboltable);
	quattable[end].op1info.d = -1; quattable[end].op1info.h = 0;
	quattable[end].op2info.d = -1; quattable[end].op2info.h = 0;
	quattable[end].resinfo.d = -1; quattable[end].resinfo.h = 0;
	if (!outLiveVar.empty())
	{
		set<string>::iterator it;
		for (it = outLiveVar.begin(); it != outLiveVar.end();)
		{
			for (int i = 0; i < symboltable.size(); i++)
			{
				if (symboltable[i].name == (*it))
				{
					symboltable[i].info.h = 1;
				}
			}
			it++;
		}
	}
	for (int i = end; i >= begin; i--)
	{
		if (quattable[i].op != "j")
		{
			//把符号表中左值的待用信息和活跃信息附加到四元式的左值info上;
			for (int j = 0; j < symboltable.size(); j++)
			{
				if (symboltable[j].name == quattable[i].res)
				{
					quattable[i].resinfo = symboltable[j].info;
					symboltable[j].info.d = -1;
					symboltable[j].info.h = 0;//把符号表中左值的info清空
				}
			}
			//把符号表中操作数的待用信息和活跃信息附加到四元式的操作数info上;
			for (int j = 0; j < symboltable.size(); j++)
			{
				if (symboltable[j].name == quattable[i].op1)
				{
					quattable[i].op1info = symboltable[j].info;
					symboltable[j].info.h = 1;
					symboltable[j].info.d = i;
				}
				else if (symboltable[j].name == quattable[i].op2)
				{
					quattable[i].op2info = symboltable[j].info;
					symboltable[j].info.h = 1;
					symboltable[j].info.d = i;//把符号表中操作数的待用信息设为i，活跃信息设为1;
				}
			}
			//增删入口活跃信息
			for (int k = 0; k < symboltable.size(); k++)
			{
				if (quattable[i].op1 == symboltable[k].name)
					inLiveVar.insert(quattable[i].op1);
				if (quattable[i].op2 == symboltable[k].name)
					inLiveVar.insert(quattable[i].op2);
			}
			if (inLiveVar.count(quattable[i].res) != 0)
			{
				inLiveVar.erase(quattable[i].res);
			}
		}
	}
}

stack<BasicBlock> blockstack;
stack<set<string>> instack;
map<int, BasicBlock>::iterator it;
void fillall(map<int, BasicBlock>& flowGragh)
{
	it = flowGragh.end();
	it--;
	//(*it).second.predecessors.size()
	fillBlockdhinfo((*it).second.begin, (*it).second.end, (*it).second.outLiveVar, (*it).second.inLiveVar);
	for (int i = 0; i < (*it).second.predecessors.size(); i++)
		blockstack.push(flowGragh[(*it).second.predecessors[i]]);
	//cout << blockstack.top().end<<" ";
	for (int i = 0; i < (*it).second.predecessors.size(); i++)
		instack.push((*it).second.inLiveVar);
	while ((!blockstack.empty()) && (!instack.empty()))
	{
		BasicBlock temp = blockstack.top();
		//cout << blockstack.top().end << " ";
		blockstack.pop();
		set<string> temp2 = instack.top();
		instack.pop();
		fillBlockdhinfo(temp.begin, temp.end, temp2, temp.inLiveVar);
		for (int i = 0; i < temp.predecessors.size(); i++)
		{
			blockstack.push(flowGragh[temp.predecessors[i]]);
			instack.push(temp.inLiveVar);
		}
	}
}
bool isinsymboltable(string abc)
{
	for (int k = 0; k < symboltable.size(); k++)
	{
		if (abc == symboltable[k].name)
			return true;
	}
	return false;
}
int findinsymboltable(string a)
{
	for (int k = 0; k < symboltable.size(); k++)
	{
		//cout << symboltable.size();
		if (a == symboltable[k].name)
			return k;
	}
}
int existinset(set<int> b)
{
	set<int>::iterator it;

	for (it = b.begin(); it != b.end(); it++)
	{
		if ((*it) > -1)
			return (*it);
	}
	return -1;
}
vector<set<string>> RValue;//寄存器使用情况 下标为寄存器编号 内容为寄存器中所存变量
vector<set<int>> AValue;//变量存在哪
int getemptyreg()
{
	for (int i = 0; i < RValue.size(); i++)
	{
		if (RValue[i].empty())
			return i;
	}
	return -1;
}
//get寄存器函数
int getreg(quat tempquat)
{
	int a1 = existinset(AValue[findinsymboltable(tempquat.res)]);
	if (a1 > -1)
		return a1;
	else if (!isinsymboltable(tempquat.op1))
		return getemptyreg();
	else
	{
		int b1 = existinset(AValue[findinsymboltable(tempquat.op1)]);
		if (b1 > -1 && (RValue[b1].size() == 1))
		{
			if (tempquat.op1 == tempquat.res)
				return b1;
			else if (tempquat.op1info.h == 0)
				return b1;
			else
				return getemptyreg();
		}
		return getemptyreg();
	}
}

//汇编指令数据结构
struct Assembly {
	string op;
	string des;
	string arg1;
	string arg2;
	string label;
};
vector<Assembly> assemblyTable;

//2021 1.12
ofstream out("code.asm");
void outAssembly(vector<Assembly> assemblyTable)
{
	for (int i = 0; i < assemblyTable.size(); i++)
		if (assemblyTable[i].op.empty())
			out << assemblyTable[i].label << ":";
		else if (assemblyTable[i].label == "_start")
		{
			if (assemblyTable[i].arg2 != "")
				out << assemblyTable[i].label << ":  " << assemblyTable[i].op << " " << assemblyTable[i].des << ", " << assemblyTable[i].arg1 << ", " << assemblyTable[i].arg2 << endl;
			else if (assemblyTable[i].arg1 != "")
				out << assemblyTable[i].label << ":  " << assemblyTable[i].op << " " << assemblyTable[i].des << ", " << assemblyTable[i].arg1 << endl;
			else
				out << assemblyTable[i].label << ":  " << assemblyTable[i].op << " " << assemblyTable[i].des << endl;
		}
		else if (!assemblyTable[i].label.empty())
		{
			if (assemblyTable[i].arg2 != "")
				out << assemblyTable[i].label << ":      " << assemblyTable[i].op << " " << assemblyTable[i].des << ", " << assemblyTable[i].arg1 << ", " << assemblyTable[i].arg2 << endl;
			else if (assemblyTable[i].arg1 != "")
				out << assemblyTable[i].label << ":      " << assemblyTable[i].op << " " << assemblyTable[i].des << ", " << assemblyTable[i].arg1 << endl;
			else
				out << assemblyTable[i].label << ":		 " << assemblyTable[i].op << " " << assemblyTable[i].des << endl;
		}
		else
		{
			if (assemblyTable[i].arg2 != "")
				out << "		 " << assemblyTable[i].op << " " << assemblyTable[i].des << ", " << assemblyTable[i].arg1 << ", " << assemblyTable[i].arg2 << endl;
			else if (assemblyTable[i].op == "j")
				out << "		 " << assemblyTable[i].op << " " << assemblyTable[i].des << endl;
			else if (assemblyTable[i].arg1 != "")
				out << "		 " << assemblyTable[i].op << " " << assemblyTable[i].des << ", " << assemblyTable[i].arg1 << endl;
			else
				out << "		 " << assemblyTable[i].op << " " << assemblyTable[i].des << endl;
		}
}
void output(vector<Assembly> assemblyTable, vector<variable> variableTable) //vector<symbol> symboltable
{
	out << "         .data" << endl;
	//for (int i = 0; i < symboltable.size(); i++)
	for (int i = 0; i < variableTable.size(); i++)
	{
		//if (symboltable[i].name.find("t")==string::npos)
		out << variableTable[i].name << ":       " << ".word" << "  0" << endl;
		//symboltable[i].type << symboltable[i].info.d << symboltable[i].length << endl;
	}
	out << "         .text" << endl;
	outAssembly(assemblyTable);
	out << "      done";
	out.close();
}

map<int, string> label = { {0,"_start"},{1,"L1"},{2,"L2" } ,{ 3, "L3" },{ 4, "L4" },{ 5, "L5" },{ 6, "L6" },{ 7, "L7" },{ 8, "L8" },{ 9, "L9" } };
map<string, string> optable = { {"+","add"}, {"-","sub"}, {"*","mult"},{"/","div" } };
map<int, string> regtable = { {0,"$1"},{1,"$2"},{2,"$3"},{3,"$4"},{4,"$5"},{5,"$6"},{6,"$7"},{7,"$8"} };
vector<Assembly> assemblyCode;
//总控函数
void genAssembly()
{
	Assembly assembly;
	for (int i = 0; i < symboltable.size(); i++)
	{
		set<int> temp = {};
		AValue.push_back(temp);
	}
	for (int i = 0; i < 8; i++)
	{
		set<string> temp = {};
		RValue.push_back(temp);
	}
	for (int k = 0; k <= flowGragh.size() - 1; k++)
	{
		for (int i = flowGragh[k].begin; i <= flowGragh[k].end; i++)
		{
			//2021 1.12
			if (i == flowGragh[k].begin)
				assembly.label = label[flowGragh[quattable[i].seq].seq];
			else
				assembly.label.clear();
			if (quattable[i].op == "j")
			{

				//cout << "j  " << label[flowGragh[stoi(quattable[i].res)].seq] << endl;
				assembly.op = "j";
				assembly.arg1 = "";
				assembly.arg2 = "";
				assembly.des = label[flowGragh[stoi(quattable[i].res)].seq];
				assemblyTable.push_back(assembly);

			}//无条件跳转
			else if (quattable[i].op == "jz")
			{
				int temp1;
				int temp2;
				if (!isinsymboltable(quattable[i].op1))
				{
					temp1 = getemptyreg();
					//cout << "addi  " << regtable[temp1] << "  " << "$0  " << quattable[i].op1 << endl;
					assembly.op = "addi";
					assembly.arg1 = "$0";
					assembly.arg2 = quattable[i].op1;
					assembly.des = regtable[temp1];
					assemblyTable.push_back(assembly);
				}
				else
				{
					set<int>::iterator it;
					it = AValue[findinsymboltable(quattable[i].op1)].end();
					it--;
					if ((*it) > -1)
					{
						temp1 = (*it);
					}
					else
					{
						temp1 = getemptyreg();
						//cout << "lw  " << regtable[temp1] << "  " << quattable[i].op1 << endl;
						assembly.op = "lw";
						assembly.arg1 = quattable[i].op1 + "($0)";
						assembly.arg2 = "";
						assembly.des = regtable[temp1];
						assemblyTable.push_back(assembly);

						AValue[findinsymboltable(quattable[i].op1)].insert(temp1);
						RValue[temp1].insert(quattable[i].op1);
					}
				}
				if (!isinsymboltable(quattable[i].op2))
				{
					temp2 = getemptyreg();
					//cout << "addi  " << regtable[temp2] << "  " << "$0  " << quattable[i].op2 << endl;
					assembly.op = "addi";
					assembly.arg1 = "$0";
					assembly.arg2 = quattable[i].op2;
					assembly.des = regtable[temp2];
					assemblyTable.push_back(assembly);
				}
				else
				{
					set<int>::iterator it;
					it = AValue[findinsymboltable(quattable[i].op2)].end();
					it--;
					if ((*it) > -1)
					{
						temp2 = (*it);
					}
					else
					{
						temp2 = getemptyreg();
						//cout << "lw  " << regtable[temp2] << "  " << quattable[i].op2 << endl;
						assembly.op = "lw";
						assembly.arg1 = quattable[i].op2 + "($0)";
						assembly.arg2 = "";
						assembly.des = regtable[temp2];
						assemblyTable.push_back(assembly);
						AValue[findinsymboltable(quattable[i].op2)].insert(temp2);
						RValue[temp2].insert(quattable[i].op2);
					}
				}
				//cout << "beq  " << regtable[temp1] << "  " << regtable[temp2] << "  " << 4 * (stoi(quattable[i].res) - i - 1) << endl;
				assembly.op = "beq";
				assembly.arg1 = regtable[temp2];
				assembly.arg2 = to_string(4 * (stoi(quattable[i].res) - i - 1));
				assembly.des = regtable[temp1];
				assemblyTable.push_back(assembly);

			}//jz跳转
			else if (quattable[i].op != "@")
			{
				int a = existinset(AValue[findinsymboltable(quattable[i].res)]);//res在寄存器里就返回寄存器编号 如果不在就返回-1
				int b;
				int c;
				int tempreg = getreg(quattable[i]);
				if (isinsymboltable(quattable[i].op1))//如果op1在符号表里
				{
					int symbolnumop1 = findinsymboltable(quattable[i].op1);
					b = existinset(AValue[symbolnumop1]);//op1在寄存器里就返回寄存器编号 如果不在就返回-1
				}
				else if (quattable[i].op1 != "@")
				{
					b = -2;
				}//对 abc -2表示立即数
				else
				{
					b = -3;
				}//对 abc -3表示无操作数@

				if (isinsymboltable(quattable[i].op2))//如果op2在符号表里
				{
					int symbolnumop2 = findinsymboltable(quattable[i].op2);
					c = existinset(AValue[symbolnumop2]);//op2在寄存器里就返回寄存器编号 如果不在就返回-1
				}
				else if (quattable[i].op2 != "@")
				{
					c = -2;
				}//对 abc -2表示立即数
				else
				{
					c = -3;
				}//对 abc -3表示无操作数@

				if (b == tempreg)
				{
					if (c == -2)
					{
						int tempreg2 = getemptyreg();
						//cout << "addi" << "  " << regtable[tempreg2] << "  " << "$0  " << quattable[i].op2 << endl;
						assembly.op = "addi";
						assembly.arg1 = "$0";
						assembly.arg2 = quattable[i].op2;
						assembly.des = regtable[tempreg2];
						assemblyTable.push_back(assembly);

						if (quattable[i].op == "+" || quattable[i].op == "-")
						{
							//cout << optable[quattable[i].op] << "  " << regtable[b] << "  " << regtable[b] << "  " << regtable[tempreg2] << endl;
							assembly.op = optable[quattable[i].op];
							assembly.arg1 = regtable[b];
							assembly.arg2 = regtable[tempreg2];
							assembly.des = regtable[b];
							assembly.label.clear();
							assemblyTable.push_back(assembly);
							RValue[b].clear();
							RValue[b].insert(quattable[i].res);
							AValue[findinsymboltable(quattable[i].op1)].erase(b);
							AValue[findinsymboltable(quattable[i].res)].insert(b);
						}
						else if (quattable[i].op == "*" || quattable[i].op == "/")
						{
							//cout << optable[quattable[i].op] << "  " << regtable[b] << "  " << regtable[tempreg2] << endl;
							assembly.op = optable[quattable[i].op];
							assembly.arg1 = regtable[tempreg2];
							assembly.arg2 = "";
							assembly.des = regtable[b];
							assembly.label.clear();
							assemblyTable.push_back(assembly);
							//cout << "mflo" << " " << regtable[b] << endl;
							assembly.op = "mflo";
							assembly.arg1 = "";
							assembly.arg2 = "";
							assembly.des = regtable[b];
							assembly.label.clear();
							assemblyTable.push_back(assembly);
							RValue[b].clear();
							RValue[b].insert(quattable[i].res);
							AValue[findinsymboltable(quattable[i].op1)].erase(b);
							AValue[findinsymboltable(quattable[i].res)].insert(b);
						}
					}
					else if (quattable[i].op == "=" && (b == -2) && (c == -3) && (a == -1))
					{
						//cout << "lw  " << regtable[tempreg] << "  " << quattable[i].res << endl;
						assembly.op = "lw";
						assembly.arg1 = quattable[i].res + "($0)";
						assembly.arg2 = "";
						assembly.des = regtable[tempreg];
						assemblyTable.push_back(assembly);
						//cout << "addi" << "  " << regtable[tempreg] << "  " << "$0  " << quattable[i].op1 << endl;
						assembly.op = "addi";
						assembly.arg1 = "$0";
						assembly.arg2 = quattable[i].op1;
						assembly.des = regtable[tempreg];
						assembly.label.clear();
						assemblyTable.push_back(assembly);
						RValue[tempreg].insert(quattable[i].res);
						AValue[findinsymboltable(quattable[i].res)].insert(-1);
						AValue[findinsymboltable(quattable[i].res)].insert(tempreg);
					}
					else if (quattable[i].op == "=" && (b > -1) && (c == -3) && (a == -1))
					{
						RValue[tempreg].clear();
						RValue[tempreg].insert(quattable[i].res);
						AValue[findinsymboltable(quattable[i].op1)].erase(tempreg);
						AValue[findinsymboltable(quattable[i].res)].insert(tempreg);
					}
					else if (quattable[i].op == "=" && (b == -2) && (c == -3) && (a > -1))
					{
						//cout << "addi" << "  " << regtable[a] << "  " << "$0  " << quattable[i].op1 << endl;
						assembly.op = "addi";
						assembly.arg1 = "$0";
						assembly.arg2 = quattable[i].op1;
						assembly.des = regtable[a];
						assemblyTable.push_back(assembly);
					}
					else if (quattable[i].op == "=" && (b > -1) && (c == -3) && (a > -1))
					{
						if (a != b)
						{
							//cout << "add" << "  " << regtable[a] << "  " << "$0  " << regtable[b] << endl;
							assembly.op = "add";
							assembly.arg1 = "$0";
							assembly.arg2 = regtable[b];
							assembly.des = regtable[a];
							assemblyTable.push_back(assembly);
						}
						else
						{
							RValue[tempreg].clear();
							RValue[tempreg].insert(quattable[i].res);
							AValue[findinsymboltable(quattable[i].op1)].erase(tempreg);
							AValue[findinsymboltable(quattable[i].res)].insert(tempreg);

						}

					}

				}
				else if (b != tempreg)
				{
					if (c == -2 && a == -1 && b == -1)
					{
						//cout << "lw  " << regtable[tempreg] << "  " << quattable[i].op1 << endl;
						assembly.op = "lw";
						assembly.arg1 = quattable[i].op1 + "($0)";
						assembly.arg2 = "";
						assembly.des = regtable[tempreg];
						assemblyTable.push_back(assembly);
						RValue[tempreg].clear();
						RValue[tempreg].insert(quattable[i].op1);
						AValue[findinsymboltable(quattable[i].op1)].insert(tempreg);
						//AValue[findinsymboltable(quattable[i].res)].insert(b);
						int tempreg2 = getemptyreg();
						//cout << "addi" << "  " << regtable[tempreg2] << "  " << "$0  " << quattable[i].op2 << endl;
						assembly.op = "addi";
						assembly.arg1 = "$0";
						assembly.arg2 = quattable[i].op2;
						assembly.des = regtable[tempreg2];
						assembly.label.clear();
						assemblyTable.push_back(assembly);
						if (quattable[i].op == "+" || quattable[i].op == "-")
						{
							//cout << optable[quattable[i].op] << "  " << regtable[tempreg] << "  " << regtable[tempreg] << "  " << regtable[tempreg2] << endl;
							assembly.op = optable[quattable[i].op];
							assembly.arg1 = regtable[tempreg];
							assembly.arg2 = regtable[tempreg2];
							assembly.des = regtable[tempreg];
							assembly.label.clear();
							assemblyTable.push_back(assembly);
							RValue[tempreg].clear();
							RValue[tempreg].insert(quattable[i].res);
							AValue[findinsymboltable(quattable[i].res)].insert(tempreg);
						}
						else if (quattable[i].op == "*" || quattable[i].op == "/")
						{
							//cout << optable[quattable[i].op] << "  " << regtable[tempreg] << "  " << regtable[tempreg2] << endl;
							assembly.op = optable[quattable[i].op];
							assembly.arg1 = regtable[tempreg2];
							assembly.arg2 = "";
							assembly.des = regtable[tempreg];
							assembly.label.clear();
							assemblyTable.push_back(assembly);
							//cout << "mflo" << " " << regtable[tempreg] << endl;
							assembly.op = "mflo";
							assembly.arg1 = "";
							assembly.arg2 = "";
							assembly.des = regtable[tempreg];
							assembly.label.clear();
							assemblyTable.push_back(assembly);
							RValue[tempreg].clear();
							RValue[tempreg].insert(quattable[i].res);
							AValue[findinsymboltable(quattable[i].res)].insert(b);
						}
					}
					else if (quattable[i].op == "=" && (b == -2) && (c == -3) && (a == -1))
					{
						//cout << "lw  " << regtable[tempreg] << "  " << quattable[i].res << endl;
						assembly.op = "lw";
						assembly.arg1 = quattable[i].res + "($0)";
						assembly.arg2 = "";
						assembly.des = regtable[tempreg];
						assemblyTable.push_back(assembly);
						//cout << "addi" << "  " << regtable[tempreg] << "  " << "$0  " << quattable[i].op1 << endl;
						assembly.op = "addi";
						assembly.arg1 = "$0";
						assembly.arg2 = quattable[i].op1;
						assembly.des = regtable[tempreg];
						assembly.label.clear();
						assemblyTable.push_back(assembly);
						RValue[tempreg].insert(quattable[i].res);
						AValue[findinsymboltable(quattable[i].res)].insert(-1);
						AValue[findinsymboltable(quattable[i].res)].insert(tempreg);
					}
					else if (quattable[i].op == "=" && (b == -2) && (c == -3) && (a > -1))
					{
						//cout << "addi" << "  " << regtable[a] << "  " << "$0  " << quattable[i].op1 << endl;
						assembly.op = "addi";
						assembly.arg1 = "$0";
						assembly.arg2 = quattable[i].op1;
						assembly.des = regtable[a];
						assemblyTable.push_back(assembly);
					}
					else if (quattable[i].op == "=" && (b > -1) && (c == -3) && (a > -1))
					{
						//cout << "add" << "  " << regtable[a] << "  " << "$0  " << regtable[b] << endl;
						assembly.op = "add";
						assembly.arg1 = "$0";
						assembly.arg2 = regtable[b];
						assembly.des = regtable[a];
						assemblyTable.push_back(assembly);

					}
				}
			}
			else
			{
				assembly.label = label[flowGragh[k].seq];
				assembly.arg1.clear();
				assembly.arg2.clear();
				assembly.des.clear();
				assembly.op.clear();
				assemblyTable.push_back(assembly);
			}

		}
	}
}

int counts = 0;
symbol qgf;
void tableAnalyse(stack<token>& mtokenstack, stack<int>& mnstack, stack<string>& subpstack, vector<string>& moutbuffer, token t, table2*& GOTO2, int& countint, int& countsymbol, int& counts)
{
	int count = 0;
	for (int i = 0; i < GOTO2[mnstack.top()].tg.size(); i++)
	{
		if (t.type == GOTO2[mnstack.top()].tg[i])
		{
			if (GOTO2[mnstack.top()].next[i] > 0)
			{
				cout << "	转移到状态:" << GOTO2[mnstack.top()].next[i] << endl;
				mnstack.push(GOTO2[mnstack.top()].next[i]);
				if (t.type != "epsilon")
				{
					mtokenstack.push(t);
					moutbuffer.push_back(t.name);
				}
				break;
			}
			else if (GOTO2[mnstack.top()].next[i] < 0)
			{
				int num = 0 - GOTO2[mnstack.top()].next[i];
				cout << "使用产生式规约:" << productions[num].leftp << "--->";
				if (productions[num].ifsubp)
				{
					quat temp = quatdeal(productions[num].subp);
					if (temp.num == 0 && temp.op == "1")
					{
						token m = mtokenstack.top();
						subpstack.push(m.name);
						cout << endl; cout << "放入栈" << m.name << endl;
					}
					else if (temp.num == 0 && temp.op == "2")
					{
						token m = mtokenstack.top();
						qgf.type = m.name;
						if (m.name == "int" && countint == 0)
							countint++;
						else if (m.name == "int" && countint != 0)
						{
							qgf.length = 4;
							qgf.offset = offset;
							offset += qgf.length;
							cout << endl;
							cout << "声明变量为:" << qgf.type << " / " << qgf.length << " / " << qgf.offset << endl;
						}
						else if (m.name == "float")
						{
							qgf.length = 4;
							qgf.offset = offset;
							offset += qgf.length;
							cout << endl;
							cout << "声明变量为:" << qgf.type << " / " << qgf.length << " / " << qgf.offset << endl;
						}
					}
					else if (temp.num == 0 && temp.op == "3")
					{
						token temp = mtokenstack.top();
						mtokenstack.pop();
						token m = mtokenstack.top();
						mtokenstack.push(temp);
						qgf.name = m.name;
						symboltable.push_back(qgf);
						//2021 1.12
						variable tempVar;
						tempVar.type = qgf.type;
						tempVar.name = qgf.name;
						variableTable.push_back(tempVar);

						//cout << "push符号表:" << qgf.type << " / " << qgf.name << " / " << qgf.length << " / " << qgf.offset << endl;
						countsymbol++;
					}
					//2021 1.8
					else if (temp.num == 0 && temp.op == "4")
					{

						string qgf = subpstack.top();
						subpstack.pop();
						string qgf2 = subpstack.top();
						subpstack.push(qgf);
						temp.seq = countquat;
						temp.op2 = qgf;
						temp.op1 = qgf2;
						temp.op = "jz";
						temp.res = "0";
						quattable.push_back(temp);
						TC.push(temp.seq);
						cout << endl; cout << "四元式：" << "序号" << temp.seq << "(" << temp.op << ", " << temp.op1 << ", " << temp.op2 << ", " << temp.res << ")" << endl;
						countquat++;
						temp.seq = countquat;
						temp.op = "j";
						temp.op2 = "@";
						temp.op1 = "@";
						temp.res = "0";
						quattable.push_back(temp);
						FC.push(temp.seq);
						cout << endl; cout << "四元式：" << "序号" << temp.seq << "(" << temp.op << ", " << temp.op1 << ", " << temp.op2 << ", " << temp.res << ")" << endl;
						countquat++;
					}
					else if (temp.num == 0 && temp.op == "5")
					{
						quattable[TC.top()].res = to_string(countquat);
						//cout << endl;
						//cout << TC.top() << endl;
						//cout << quattable[TC.top()].res << endl;
						TC.pop();
					}
					else if (temp.num == 0 && temp.op == "6")
					{
						temp.seq = countquat;
						temp.op = "j";
						temp.res = "0";
						quattable.push_back(temp);
						cout << endl; cout << "四元式：" << "序号" << temp.seq << "(" << temp.op << ", " << temp.op1 << ", " << temp.op2 << ", " << temp.res << ")" << endl;
						TC.push(temp.seq);
						;							countquat++;
						quattable[FC.top()].res = to_string(countquat);
						//cout << endl;
						//cout << FC.top() << endl;
						//cout << quattable[FC.top()].res << endl;
						FC.pop();
					}
					else if (temp.num == 0 && temp.op == "7")
					{
						quattable[TC.top()].res = to_string(countquat);
						//cout << endl;
						//cout << TC.top() << endl;
						//cout << quattable[TC.top()].res << endl;
						TC.pop();
					}

					else if (temp.num == 1 && temp.op == "=")
					{
						temp.op1 = subpstack.top();
						//2021 1.8
						temp.seq = countquat;
						subpstack.pop();
						temp.res = subpstack.top();
						quattable.push_back(temp);
						//2021 1.8
						countquat++;
						cout << endl; cout << "四元式：" << "序号" << temp.seq << "(" << temp.op << ", " << temp.op1 << ", " << temp.op2 << ", " << temp.res << ")" << endl;
					}
					else if (temp.num == 1 && temp.op == "-")
					{
						temp.res = newTemp();
						temp.seq = countquat;
						temp.op1 = subpstack.top();
						subpstack.pop();
						quattable.push_back(temp);
						subpstack.push(temp.res);
						countquat++;
						cout << endl; cout << "四元式：" << "序号" << temp.seq << "(" << temp.op << ", " << temp.op1 << ", " << temp.op2 << ", " << temp.res << ")" << endl;
						qgf.name = temp.res;
						for (int i = 0; i < symboltable.size(); i++)
						{
							if (symboltable[i].name == temp.op1)
							{
								qgf.type = symboltable[i].type;
								qgf.length = symboltable[i].length;
								qgf.offset = offset;
								symboltable.push_back(qgf);
								offset += qgf.length;
							}
						}
						countsymbol++;
					}
					else
					{
						temp.seq = countquat;
						temp.op2 = subpstack.top();
						subpstack.pop();
						temp.op1 = subpstack.top();
						subpstack.pop();
						temp.res = newTemp();
						quattable.push_back(temp);
						subpstack.push(temp.res);
						countquat++;
						cout << endl; cout << "四元式：" << "序号" << temp.seq << "(" << temp.op << ", " << temp.op1 << ", " << temp.op2 << ", " << temp.res << ")" << endl;
						qgf.name = temp.res;
						for (int i = 0; i < symboltable.size(); i++)
						{
							if (symboltable[i].name == temp.op1)
							{
								qgf.type = symboltable[i].type;
								qgf.length = symboltable[i].length;
								qgf.offset = offset;
								symboltable.push_back(qgf);
								offset += qgf.length;
							}
						}
						countsymbol++;
					}
				}
				for (int i = 0; i < productions[num].numofrights; i++)
				{
					cout << productions[num].rightp[i];
					if (productions[num].rightp[0] != "epsilon")
					{
						mtokenstack.pop();
						moutbuffer.pop_back();
					}
					mnstack.pop();
					isused = false;
				}
				token a{ productions[num].leftp ,productions[num].leftp };
				tableAnalyse(mtokenstack, mnstack, subpstack, moutbuffer, a, GOTO2, countint, countsymbol, counts);
				counts++;
				break;
			}
			else
			{
				cout << "编译完成" << endl;
				/*
				divideBasicBlock(quattable);
				genFlowgragh(firstnum);
				for (set<int>::iterator it = firstnum.begin(); it != firstnum.end(); it++)
				{
					cout << (*it) << endl;
				}
				map<int, BasicBlock>::iterator iter;
				for (iter = flowGragh.begin(); iter != flowGragh.end(); iter++) {

					cout << (*iter).first << " : " << (*iter).second.begin << "    " << (*iter).second.end;
					cout << "后继数量:" << (*iter).second.successors.size();
					for (int i = 0;i < (*iter).second.successors.size();i++)
						cout << "   " << (*iter).second.successors[i];
					cout << "前驱数量:" << (*iter).second.predecessors.size();
					for (int i = 0;i < (*iter).second.predecessors.size();i++)
						cout << "   " << (*iter).second.predecessors[i];
					cout << endl;
				}
				*/

				//2021 1.15
				/*
				for (vector<quat>::iterator it = quattable.begin(); it != quattable.end(); it++)
				{
					//2021 1.8
					cout << (*it).seq << "   ("; cout << (*it).op << " ,"; cout << (*it).op1 << " ,"; cout << (*it).op2 << " ,"; cout << (*it).res << ")" << endl;
				}
				*/

				//2021 1.9
				divideBasicBlock(quattable);
				genFlowgragh(firstnum);
				quat temp;
				temp.op = "@";
				temp.op1 = "@";
				temp.op2 = "@";
				temp.res = "@";
				temp.seq = quattable.size();
				quattable.push_back(temp);

				//2021 1.15
				/*
				for (set<int>::iterator it = firstnum.begin(); it != firstnum.end(); it++)
				{
					cout << (*it) << endl;
				}
				
				map<int, BasicBlock>::iterator iter;
				for (iter = flowGragh.begin(); iter != flowGragh.end(); iter++) {

					cout << (*iter).first << " : " << (*iter).second.begin << "    " << (*iter).second.end;
					cout << "后继数量:" << (*iter).second.successors.size();
					for (int i = 0; i < (*iter).second.successors.size(); i++)
						cout << "   " << (*iter).second.successors[i];
					cout << "前驱数量:" << (*iter).second.predecessors.size();
					for (int i = 0; i < (*iter).second.predecessors.size(); i++)
						cout << "   " << (*iter).second.predecessors[i];
					cout << endl;
				}

				for (int i = 0; i < symboltable.size(); i++)
				{
					//2021 1.10
					cout << symboltable[i].type << "   " << symboltable[i].name << "   " << symboltable[i].length << "   " << symboltable[i].offset << endl;//符号表打印
				}
				*/


				//fillBlockdhinfo(5, 9, flowGragh[5].outLiveVar, flowGragh[5].inLiveVar);
				//fillBlockdhinfo(4, 4, flowGragh[4].outLiveVar, flowGragh[4].inLiveVar);
				//fillBlockdhinfo(0, 3, flowGragh[0].outLiveVar, flowGragh[0].inLiveVar);
				//fillBlockdhinfo(10, 13, flowGragh[10].outLiveVar, flowGragh[10].inLiveVar);
				//fillBlockdhinfo(14, 14, flowGragh[14].outLiveVar, flowGragh[14].inLiveVar);
				fillall(flowGragh);
				for (vector<quat>::iterator it = quattable.begin(); it != quattable.end(); it++)
				{
					//2021 1.8
					cout << endl;
					cout << (*it).seq << "   ("; cout << (*it).op << " ,"; cout << (*it).op1 << " ,"; cout << (*it).op2 << " ,"; cout << (*it).res << ")" << "活跃状态:" << (*it).op1info.h << " 待用信息:" << (*it).op1info.d << "活跃状态:" << (*it).op2info.h << " 待用信息:" << (*it).op2info.d << "活跃状态:" << (*it).resinfo.h << " 待用信息:" << (*it).resinfo.d << endl;
				}
				set<string>::iterator it;
				for (it = flowGragh[5].outLiveVar.begin(); it != flowGragh[5].outLiveVar.end(); it++)
				{
					//cout << "5deout" << endl;
					//cout << (*it) << endl;
				}
				genAssembly();
				//2021 1.12
				output(assemblyTable, variableTable);
				ofstream suss("feedback.txt");
				suss << "success";
				suss.close();
				exit(0);
				//break;
			}
		}
		if (GOTO2[mnstack.top()].tg[i] == "epsilon")
		{
			isepsilon = true;
			//counts--;
		}
		if (i == GOTO2[mnstack.top()].tg.size() - 1)
		{
			//token temp;
			if (isepsilon == true)
			{
				token b{ "epsilon","epsilon" };
				isepsilon = false;
				tableAnalyse(mtokenstack, mnstack, subpstack, moutbuffer, b, GOTO2, countint, countsymbol, counts);
				isused = false;
				break;
			}
			//cout << endl;
			//cout << GOTO2[mnstack.top()].tg[0] << "  ";
			//cout << counts << endl;
			//cout << "fail in compiling." << endl;
			ofstream err("feedback.txt");
			err << "fail in compiling";
			err.close();
			exit(1);
		}
	}
	isepsilon = false;
}

int main()
{
	map<int, string> I2S;
	I2S[1] = "ASSIGN"; I2S[2] = "COMMA"; I2S[3] = "DIVIDE"; I2S[4] = "DOT"; I2S[5] = "ELSE"; I2S[6] = "EQUAL"; I2S[7] = "FLOAT"; I2S[8] = "IF"; I2S[9] = "INT"; I2S[10] = "LBRACE"; I2S[11] = "LBRACE"; I2S[12] = "LPAR"; I2S[13] = "MINUS"; I2S[14] = "NAME"; I2S[15] = "NUMBER"; I2S[16] = "PLUS"; I2S[17] = "RBRACE"; I2S[18] = "RBRACE"; I2S[19] = "RETURN"; I2S[20] = "RPAR"; I2S[21] = "SEMICOLON"; I2S[22] = "STRUCT"; I2S[23] = "TIMES";
	fstream input;
	char content[BUFF];
	input.open("token.txt", ios::in);
	token nowtoken;
	while (!input.eof()) {
		input.getline(content, BUFF);
		char* p;
		p = strtok(content, ", ");
		nowtoken.type = I2S.at(atoi(p));
		p = strtok(NULL, ", ");
		nowtoken.name = p;
		tokenqueue.push(nowtoken);
	}
	token endtoken;
	endtoken.name = "#";
	endtoken.type = "#";
	tokenqueue.push(endtoken);
	int num = 226;
	table2* GOTO2 = new table2[num];
	GOTO2[0].tg.push_back("FLOAT");
	GOTO2[0].tg.push_back("INT");
	GOTO2[0].tg.push_back("STRUCT");
	GOTO2[0].tg.push_back("declaration");
	GOTO2[0].tg.push_back("declarations");
	GOTO2[0].tg.push_back("epsilon");
	GOTO2[0].tg.push_back("fun_declaration");
	GOTO2[0].tg.push_back("program");
	GOTO2[0].tg.push_back("type");
	GOTO2[0].tg.push_back("var_declaration");
	GOTO2[0].next.push_back(1);
	GOTO2[0].next.push_back(2);
	GOTO2[0].next.push_back(3);
	GOTO2[0].next.push_back(4);
	GOTO2[0].next.push_back(5);
	GOTO2[0].next.push_back(6);
	GOTO2[0].next.push_back(7);
	GOTO2[0].next.push_back(8);
	GOTO2[0].next.push_back(9);
	GOTO2[0].next.push_back(10);
	GOTO2[1].tg.push_back("NAME");
	GOTO2[1].tg.push_back("TIMES");
	GOTO2[1].next.push_back(-17);
	GOTO2[1].next.push_back(-17);
	GOTO2[2].tg.push_back("NAME");
	GOTO2[2].tg.push_back("TIMES");
	GOTO2[2].next.push_back(-16);
	GOTO2[2].next.push_back(-16);
	GOTO2[3].tg.push_back("LBRACE");
	GOTO2[3].next.push_back(11);
	GOTO2[4].tg.push_back("FLOAT");
	GOTO2[4].tg.push_back("INT");
	GOTO2[4].tg.push_back("STRUCT");
	GOTO2[4].tg.push_back("declaration");
	GOTO2[4].tg.push_back("epsilon");
	GOTO2[4].tg.push_back("fun_declaration");
	GOTO2[4].tg.push_back("type");
	GOTO2[4].tg.push_back("var_declaration");
	GOTO2[4].tg.push_back("declarations");
	GOTO2[4].next.push_back(1);
	GOTO2[4].next.push_back(2);
	GOTO2[4].next.push_back(3);
	GOTO2[4].next.push_back(4);
	GOTO2[4].next.push_back(6);
	GOTO2[4].next.push_back(7);
	GOTO2[4].next.push_back(9);
	GOTO2[4].next.push_back(10);
	GOTO2[4].next.push_back(12);
	GOTO2[5].tg.push_back("#");
	GOTO2[5].next.push_back(-1);
	GOTO2[6].tg.push_back("#");
	GOTO2[6].next.push_back(-3);
	GOTO2[7].tg.push_back("FLOAT");
	GOTO2[7].tg.push_back("INT");
	GOTO2[7].tg.push_back("STRUCT");
	GOTO2[7].tg.push_back("epsilon");
	GOTO2[7].next.push_back(-4);
	GOTO2[7].next.push_back(-4);
	GOTO2[7].next.push_back(-4);
	GOTO2[7].next.push_back(-4);
	GOTO2[8].tg.push_back("#");
	GOTO2[8].next.push_back(0);
	GOTO2[9].tg.push_back("NAME");
	GOTO2[9].tg.push_back("TIMES");
	GOTO2[9].next.push_back(13);
	GOTO2[9].next.push_back(14);
	GOTO2[10].tg.push_back("FLOAT");
	GOTO2[10].tg.push_back("INT");
	GOTO2[10].tg.push_back("STRUCT");
	GOTO2[10].tg.push_back("epsilon");
	GOTO2[10].next.push_back(-5);
	GOTO2[10].next.push_back(-5);
	GOTO2[10].next.push_back(-5);
	GOTO2[10].next.push_back(-5);
	GOTO2[11].tg.push_back("FLOAT");
	GOTO2[11].tg.push_back("INT");
	GOTO2[11].tg.push_back("STRUCT");
	GOTO2[11].tg.push_back("epsilon");
	GOTO2[11].tg.push_back("field");
	GOTO2[11].tg.push_back("fields");
	GOTO2[11].tg.push_back("type");
	GOTO2[11].next.push_back(1);
	GOTO2[11].next.push_back(2);
	GOTO2[11].next.push_back(3);
	GOTO2[11].next.push_back(15);
	GOTO2[11].next.push_back(16);
	GOTO2[11].next.push_back(17);
	GOTO2[11].next.push_back(18);
	GOTO2[12].tg.push_back("#");
	GOTO2[12].next.push_back(-2);
	GOTO2[13].tg.push_back("LPAR");
	GOTO2[13].tg.push_back("SEMICOLON");
	GOTO2[13].next.push_back(19);
	GOTO2[13].next.push_back(20);
	GOTO2[14].tg.push_back("NAME");
	GOTO2[14].tg.push_back("TIMES");
	GOTO2[14].next.push_back(-18);
	GOTO2[14].next.push_back(-18);
	GOTO2[15].tg.push_back("RBRACE");
	GOTO2[15].next.push_back(-21);
	GOTO2[16].tg.push_back("FLOAT");
	GOTO2[16].tg.push_back("INT");
	GOTO2[16].tg.push_back("STRUCT");
	GOTO2[16].tg.push_back("epsilon");
	GOTO2[16].tg.push_back("field");
	GOTO2[16].tg.push_back("type");
	GOTO2[16].tg.push_back("fields");
	GOTO2[16].next.push_back(1);
	GOTO2[16].next.push_back(2);
	GOTO2[16].next.push_back(3);
	GOTO2[16].next.push_back(15);
	GOTO2[16].next.push_back(16);
	GOTO2[16].next.push_back(18);
	GOTO2[16].next.push_back(21);
	GOTO2[17].tg.push_back("RBRACE");
	GOTO2[17].next.push_back(22);
	GOTO2[18].tg.push_back("TIMES");
	GOTO2[18].tg.push_back("NAME");
	GOTO2[18].next.push_back(14);
	GOTO2[18].next.push_back(23);
	GOTO2[19].tg.push_back("FLOAT");
	GOTO2[19].tg.push_back("INT");
	GOTO2[19].tg.push_back("STRUCT");
	GOTO2[19].tg.push_back("epsilon");
	GOTO2[19].tg.push_back("more_parameters");
	GOTO2[19].tg.push_back("parameter");
	GOTO2[19].tg.push_back("parameters");
	GOTO2[19].tg.push_back("type");
	GOTO2[19].next.push_back(1);
	GOTO2[19].next.push_back(2);
	GOTO2[19].next.push_back(3);
	GOTO2[19].next.push_back(24);
	GOTO2[19].next.push_back(25);
	GOTO2[19].next.push_back(26);
	GOTO2[19].next.push_back(27);
	GOTO2[19].next.push_back(28);
	GOTO2[20].tg.push_back("FLOAT");
	GOTO2[20].tg.push_back("INT");
	GOTO2[20].tg.push_back("STRUCT");
	GOTO2[20].tg.push_back("epsilon");
	GOTO2[20].next.push_back(-15);
	GOTO2[20].next.push_back(-15);
	GOTO2[20].next.push_back(-15);
	GOTO2[20].next.push_back(-15);
	GOTO2[21].tg.push_back("RBRACE");
	GOTO2[21].next.push_back(-20);
	GOTO2[22].tg.push_back("NAME");
	GOTO2[22].tg.push_back("TIMES");
	GOTO2[22].next.push_back(-19);
	GOTO2[22].next.push_back(-19);
	GOTO2[23].tg.push_back("SEMICOLON");
	GOTO2[23].next.push_back(29);
	GOTO2[24].tg.push_back("RPAR");
	GOTO2[24].next.push_back(-8);
	GOTO2[25].tg.push_back("RPAR");
	GOTO2[25].next.push_back(-7);
	GOTO2[26].tg.push_back("COMMA");
	GOTO2[26].tg.push_back("RPAR");
	GOTO2[26].next.push_back(30);
	GOTO2[26].next.push_back(-10);
	GOTO2[27].tg.push_back("RPAR");
	GOTO2[27].next.push_back(31);
	GOTO2[28].tg.push_back("TIMES");
	GOTO2[28].tg.push_back("NAME");
	GOTO2[28].next.push_back(14);
	GOTO2[28].next.push_back(32);
	GOTO2[29].tg.push_back("FLOAT");
	GOTO2[29].tg.push_back("INT");
	GOTO2[29].tg.push_back("STRUCT");
	GOTO2[29].tg.push_back("epsilon");
	GOTO2[29].next.push_back(-22);
	GOTO2[29].next.push_back(-22);
	GOTO2[29].next.push_back(-22);
	GOTO2[29].next.push_back(-22);
	GOTO2[30].tg.push_back("FLOAT");
	GOTO2[30].tg.push_back("INT");
	GOTO2[30].tg.push_back("STRUCT");
	GOTO2[30].tg.push_back("parameter");
	GOTO2[30].tg.push_back("type");
	GOTO2[30].tg.push_back("more_parameters");
	GOTO2[30].next.push_back(1);
	GOTO2[30].next.push_back(2);
	GOTO2[30].next.push_back(3);
	GOTO2[30].next.push_back(26);
	GOTO2[30].next.push_back(28);
	GOTO2[30].next.push_back(33);
	GOTO2[31].tg.push_back("LBRACE");
	GOTO2[31].tg.push_back("block");
	GOTO2[31].next.push_back(34);
	GOTO2[31].next.push_back(35);
	GOTO2[32].tg.push_back("COMMA");
	GOTO2[32].tg.push_back("RPAR");
	GOTO2[32].next.push_back(-11);
	GOTO2[32].next.push_back(-11);
	GOTO2[33].tg.push_back("RPAR");
	GOTO2[33].next.push_back(-9);
	GOTO2[34].tg.push_back("FLOAT");
	GOTO2[34].tg.push_back("INT");
	GOTO2[34].tg.push_back("STRUCT");
	GOTO2[34].tg.push_back("epsilon");
	GOTO2[34].tg.push_back("type");
	GOTO2[34].tg.push_back("var_declaration");
	GOTO2[34].tg.push_back("var_declarations");
	GOTO2[34].next.push_back(1);
	GOTO2[34].next.push_back(2);
	GOTO2[34].next.push_back(3);
	GOTO2[34].next.push_back(36);
	GOTO2[34].next.push_back(37);
	GOTO2[34].next.push_back(38);
	GOTO2[34].next.push_back(39);
	GOTO2[35].tg.push_back("FLOAT");
	GOTO2[35].tg.push_back("INT");
	GOTO2[35].tg.push_back("STRUCT");
	GOTO2[35].tg.push_back("epsilon");
	GOTO2[35].next.push_back(-6);
	GOTO2[35].next.push_back(-6);
	GOTO2[35].next.push_back(-6);
	GOTO2[35].next.push_back(-6);
	GOTO2[36].tg.push_back("IF");
	GOTO2[36].tg.push_back("LBRACE");
	GOTO2[36].tg.push_back("NAME");
	GOTO2[36].tg.push_back("RETURN");
	GOTO2[36].tg.push_back("epsilon");
	GOTO2[36].next.push_back(-14);
	GOTO2[36].next.push_back(-14);
	GOTO2[36].next.push_back(-14);
	GOTO2[36].next.push_back(-14);
	GOTO2[36].next.push_back(-14);
	GOTO2[37].tg.push_back("TIMES");
	GOTO2[37].tg.push_back("NAME");
	GOTO2[37].next.push_back(14);
	GOTO2[37].next.push_back(40);
	GOTO2[38].tg.push_back("FLOAT");
	GOTO2[38].tg.push_back("INT");
	GOTO2[38].tg.push_back("STRUCT");
	GOTO2[38].tg.push_back("epsilon");
	GOTO2[38].tg.push_back("type");
	GOTO2[38].tg.push_back("var_declaration");
	GOTO2[38].tg.push_back("var_declarations");
	GOTO2[38].next.push_back(1);
	GOTO2[38].next.push_back(2);
	GOTO2[38].next.push_back(3);
	GOTO2[38].next.push_back(36);
	GOTO2[38].next.push_back(37);
	GOTO2[38].next.push_back(38);
	GOTO2[38].next.push_back(41);
	GOTO2[39].tg.push_back("C");
	GOTO2[39].tg.push_back("IF");
	GOTO2[39].tg.push_back("LBRACE");
	GOTO2[39].tg.push_back("NAME");
	GOTO2[39].tg.push_back("RETURN");
	GOTO2[39].tg.push_back("T");
	GOTO2[39].tg.push_back("block");
	GOTO2[39].tg.push_back("epsilon");
	GOTO2[39].tg.push_back("lexp");
	GOTO2[39].tg.push_back("statement");
	GOTO2[39].tg.push_back("statements");
	GOTO2[39].tg.push_back("var");
	GOTO2[39].next.push_back(42);
	GOTO2[39].next.push_back(43);
	GOTO2[39].next.push_back(44);
	GOTO2[39].next.push_back(45);
	GOTO2[39].next.push_back(46);
	GOTO2[39].next.push_back(47);
	GOTO2[39].next.push_back(48);
	GOTO2[39].next.push_back(49);
	GOTO2[39].next.push_back(50);
	GOTO2[39].next.push_back(51);
	GOTO2[39].next.push_back(52);
	GOTO2[39].next.push_back(53);
	GOTO2[40].tg.push_back("SEMICOLON");
	GOTO2[40].next.push_back(20);
	GOTO2[41].tg.push_back("IF");
	GOTO2[41].tg.push_back("LBRACE");
	GOTO2[41].tg.push_back("NAME");
	GOTO2[41].tg.push_back("RETURN");
	GOTO2[41].tg.push_back("epsilon");
	GOTO2[41].next.push_back(-13);
	GOTO2[41].next.push_back(-13);
	GOTO2[41].next.push_back(-13);
	GOTO2[41].next.push_back(-13);
	GOTO2[41].next.push_back(-13);
	GOTO2[42].tg.push_back("IF");
	GOTO2[42].tg.push_back("NAME");
	GOTO2[42].tg.push_back("var");
	GOTO2[42].tg.push_back("C");
	GOTO2[42].tg.push_back("LBRACE");
	GOTO2[42].tg.push_back("RETURN");
	GOTO2[42].tg.push_back("T");
	GOTO2[42].tg.push_back("block");
	GOTO2[42].tg.push_back("lexp");
	GOTO2[42].tg.push_back("statement");
	GOTO2[42].next.push_back(43);
	GOTO2[42].next.push_back(45);
	GOTO2[42].next.push_back(53);
	GOTO2[42].next.push_back(54);
	GOTO2[42].next.push_back(55);
	GOTO2[42].next.push_back(56);
	GOTO2[42].next.push_back(57);
	GOTO2[42].next.push_back(58);
	GOTO2[42].next.push_back(59);
	GOTO2[42].next.push_back(60);
	GOTO2[43].tg.push_back("LPAR");
	GOTO2[43].next.push_back(61);
	GOTO2[44].tg.push_back("FLOAT");
	GOTO2[44].tg.push_back("INT");
	GOTO2[44].tg.push_back("STRUCT");
	GOTO2[44].tg.push_back("epsilon");
	GOTO2[44].tg.push_back("type");
	GOTO2[44].tg.push_back("var_declaration");
	GOTO2[44].tg.push_back("var_declarations");
	GOTO2[44].next.push_back(1);
	GOTO2[44].next.push_back(2);
	GOTO2[44].next.push_back(3);
	GOTO2[44].next.push_back(36);
	GOTO2[44].next.push_back(37);
	GOTO2[44].next.push_back(38);
	GOTO2[44].next.push_back(62);
	GOTO2[45].tg.push_back("ASSIGN");
	GOTO2[45].tg.push_back("DOT");
	GOTO2[45].tg.push_back("LBRACE");
	GOTO2[45].next.push_back(-54);
	GOTO2[45].next.push_back(-54);
	GOTO2[45].next.push_back(-54);
	GOTO2[46].tg.push_back("LPAR");
	GOTO2[46].tg.push_back("MINUS");
	GOTO2[46].tg.push_back("NAME");
	GOTO2[46].tg.push_back("NUMBER");
	GOTO2[46].tg.push_back("exp");
	GOTO2[46].tg.push_back("var");
	GOTO2[46].next.push_back(63);
	GOTO2[46].next.push_back(64);
	GOTO2[46].next.push_back(65);
	GOTO2[46].next.push_back(66);
	GOTO2[46].next.push_back(67);
	GOTO2[46].next.push_back(68);
	GOTO2[47].tg.push_back("C");
	GOTO2[47].tg.push_back("IF");
	GOTO2[47].tg.push_back("LBRACE");
	GOTO2[47].tg.push_back("NAME");
	GOTO2[47].tg.push_back("RETURN");
	GOTO2[47].tg.push_back("T");
	GOTO2[47].tg.push_back("block");
	GOTO2[47].tg.push_back("lexp");
	GOTO2[47].tg.push_back("var");
	GOTO2[47].tg.push_back("statement");
	GOTO2[47].next.push_back(42);
	GOTO2[47].next.push_back(43);
	GOTO2[47].next.push_back(44);
	GOTO2[47].next.push_back(45);
	GOTO2[47].next.push_back(46);
	GOTO2[47].next.push_back(47);
	GOTO2[47].next.push_back(48);
	GOTO2[47].next.push_back(50);
	GOTO2[47].next.push_back(53);
	GOTO2[47].next.push_back(69);
	GOTO2[48].tg.push_back("SEMICOLON");
	GOTO2[48].next.push_back(-29);
	GOTO2[49].tg.push_back("RBRACE");
	GOTO2[49].next.push_back(-24);
	GOTO2[50].tg.push_back("ASSIGN");
	GOTO2[50].tg.push_back("DOT");
	GOTO2[50].tg.push_back("LBRACE");
	GOTO2[50].next.push_back(70);
	GOTO2[50].next.push_back(71);
	GOTO2[50].next.push_back(72);
	GOTO2[51].tg.push_back("SEMICOLON");
	GOTO2[51].next.push_back(73);
	GOTO2[52].tg.push_back("RBRACE");
	GOTO2[52].next.push_back(74);
	GOTO2[53].tg.push_back("ASSIGN");
	GOTO2[53].tg.push_back("DOT");
	GOTO2[53].tg.push_back("LBRACE");
	GOTO2[53].next.push_back(-33);
	GOTO2[53].next.push_back(-33);
	GOTO2[53].next.push_back(-33);
	GOTO2[54].tg.push_back("IF");
	GOTO2[54].tg.push_back("NAME");
	GOTO2[54].tg.push_back("var");
	GOTO2[54].tg.push_back("C");
	GOTO2[54].tg.push_back("LBRACE");
	GOTO2[54].tg.push_back("RETURN");
	GOTO2[54].tg.push_back("T");
	GOTO2[54].tg.push_back("block");
	GOTO2[54].tg.push_back("lexp");
	GOTO2[54].tg.push_back("statement");
	GOTO2[54].next.push_back(43);
	GOTO2[54].next.push_back(45);
	GOTO2[54].next.push_back(53);
	GOTO2[54].next.push_back(54);
	GOTO2[54].next.push_back(55);
	GOTO2[54].next.push_back(56);
	GOTO2[54].next.push_back(57);
	GOTO2[54].next.push_back(58);
	GOTO2[54].next.push_back(59);
	GOTO2[54].next.push_back(75);
	GOTO2[55].tg.push_back("FLOAT");
	GOTO2[55].tg.push_back("INT");
	GOTO2[55].tg.push_back("STRUCT");
	GOTO2[55].tg.push_back("epsilon");
	GOTO2[55].tg.push_back("type");
	GOTO2[55].tg.push_back("var_declaration");
	GOTO2[55].tg.push_back("var_declarations");
	GOTO2[55].next.push_back(1);
	GOTO2[55].next.push_back(2);
	GOTO2[55].next.push_back(3);
	GOTO2[55].next.push_back(36);
	GOTO2[55].next.push_back(37);
	GOTO2[55].next.push_back(38);
	GOTO2[55].next.push_back(76);
	GOTO2[56].tg.push_back("LPAR");
	GOTO2[56].tg.push_back("MINUS");
	GOTO2[56].tg.push_back("NAME");
	GOTO2[56].tg.push_back("NUMBER");
	GOTO2[56].tg.push_back("exp");
	GOTO2[56].tg.push_back("var");
	GOTO2[56].next.push_back(77);
	GOTO2[56].next.push_back(78);
	GOTO2[56].next.push_back(79);
	GOTO2[56].next.push_back(80);
	GOTO2[56].next.push_back(81);
	GOTO2[56].next.push_back(82);
	GOTO2[57].tg.push_back("IF");
	GOTO2[57].tg.push_back("NAME");
	GOTO2[57].tg.push_back("var");
	GOTO2[57].tg.push_back("C");
	GOTO2[57].tg.push_back("LBRACE");
	GOTO2[57].tg.push_back("RETURN");
	GOTO2[57].tg.push_back("T");
	GOTO2[57].tg.push_back("block");
	GOTO2[57].tg.push_back("lexp");
	GOTO2[57].tg.push_back("statement");
	GOTO2[57].next.push_back(43);
	GOTO2[57].next.push_back(45);
	GOTO2[57].next.push_back(53);
	GOTO2[57].next.push_back(54);
	GOTO2[57].next.push_back(55);
	GOTO2[57].next.push_back(56);
	GOTO2[57].next.push_back(57);
	GOTO2[57].next.push_back(58);
	GOTO2[57].next.push_back(59);
	GOTO2[57].next.push_back(83);
	GOTO2[58].tg.push_back("ELSE");
	GOTO2[58].tg.push_back("SEMICOLON");
	GOTO2[58].next.push_back(-29);
	GOTO2[58].next.push_back(-29);
	GOTO2[59].tg.push_back("DOT");
	GOTO2[59].tg.push_back("LBRACE");
	GOTO2[59].tg.push_back("ASSIGN");
	GOTO2[59].next.push_back(71);
	GOTO2[59].next.push_back(72);
	GOTO2[59].next.push_back(84);
	GOTO2[60].tg.push_back("ELSE");
	GOTO2[60].tg.push_back("SEMICOLON");
	GOTO2[60].next.push_back(85);
	GOTO2[60].next.push_back(-26);
	GOTO2[61].tg.push_back("LPAR");
	GOTO2[61].tg.push_back("MINUS");
	GOTO2[61].tg.push_back("NAME");
	GOTO2[61].tg.push_back("NUMBER");
	GOTO2[61].tg.push_back("exp");
	GOTO2[61].tg.push_back("var");
	GOTO2[61].next.push_back(86);
	GOTO2[61].next.push_back(87);
	GOTO2[61].next.push_back(88);
	GOTO2[61].next.push_back(89);
	GOTO2[61].next.push_back(90);
	GOTO2[61].next.push_back(91);
	GOTO2[62].tg.push_back("C");
	GOTO2[62].tg.push_back("IF");
	GOTO2[62].tg.push_back("LBRACE");
	GOTO2[62].tg.push_back("NAME");
	GOTO2[62].tg.push_back("RETURN");
	GOTO2[62].tg.push_back("T");
	GOTO2[62].tg.push_back("block");
	GOTO2[62].tg.push_back("epsilon");
	GOTO2[62].tg.push_back("lexp");
	GOTO2[62].tg.push_back("statement");
	GOTO2[62].tg.push_back("var");
	GOTO2[62].tg.push_back("statements");
	GOTO2[62].next.push_back(42);
	GOTO2[62].next.push_back(43);
	GOTO2[62].next.push_back(44);
	GOTO2[62].next.push_back(45);
	GOTO2[62].next.push_back(46);
	GOTO2[62].next.push_back(47);
	GOTO2[62].next.push_back(48);
	GOTO2[62].next.push_back(49);
	GOTO2[62].next.push_back(50);
	GOTO2[62].next.push_back(51);
	GOTO2[62].next.push_back(53);
	GOTO2[62].next.push_back(92);
	GOTO2[63].tg.push_back("LPAR");
	GOTO2[63].tg.push_back("MINUS");
	GOTO2[63].tg.push_back("NAME");
	GOTO2[63].tg.push_back("NUMBER");
	GOTO2[63].tg.push_back("var");
	GOTO2[63].tg.push_back("exp");
	GOTO2[63].next.push_back(86);
	GOTO2[63].next.push_back(87);
	GOTO2[63].next.push_back(88);
	GOTO2[63].next.push_back(89);
	GOTO2[63].next.push_back(91);
	GOTO2[63].next.push_back(93);
	GOTO2[64].tg.push_back("LPAR");
	GOTO2[64].tg.push_back("MINUS");
	GOTO2[64].tg.push_back("NAME");
	GOTO2[64].tg.push_back("NUMBER");
	GOTO2[64].tg.push_back("var");
	GOTO2[64].tg.push_back("exp");
	GOTO2[64].next.push_back(63);
	GOTO2[64].next.push_back(64);
	GOTO2[64].next.push_back(65);
	GOTO2[64].next.push_back(66);
	GOTO2[64].next.push_back(68);
	GOTO2[64].next.push_back(94);
	GOTO2[65].tg.push_back("LPAR");
	GOTO2[65].tg.push_back("DIVIDE");
	GOTO2[65].tg.push_back("DOT");
	GOTO2[65].tg.push_back("EQUAL");
	GOTO2[65].tg.push_back("LBRACE");
	GOTO2[65].tg.push_back("MINUS");
	GOTO2[65].tg.push_back("PLUS");
	GOTO2[65].tg.push_back("SEMICOLON");
	GOTO2[65].tg.push_back("TIMES");
	GOTO2[65].next.push_back(95);
	GOTO2[65].next.push_back(-54);
	GOTO2[65].next.push_back(-54);
	GOTO2[65].next.push_back(-54);
	GOTO2[65].next.push_back(-54);
	GOTO2[65].next.push_back(-54);
	GOTO2[65].next.push_back(-54);
	GOTO2[65].next.push_back(-54);
	GOTO2[65].next.push_back(-54);
	GOTO2[66].tg.push_back("DIVIDE");
	GOTO2[66].tg.push_back("DOT");
	GOTO2[66].tg.push_back("EQUAL");
	GOTO2[66].tg.push_back("LBRACE");
	GOTO2[66].tg.push_back("MINUS");
	GOTO2[66].tg.push_back("PLUS");
	GOTO2[66].tg.push_back("SEMICOLON");
	GOTO2[66].tg.push_back("TIMES");
	GOTO2[66].next.push_back(-49);
	GOTO2[66].next.push_back(-49);
	GOTO2[66].next.push_back(-49);
	GOTO2[66].next.push_back(-49);
	GOTO2[66].next.push_back(-49);
	GOTO2[66].next.push_back(-49);
	GOTO2[66].next.push_back(-49);
	GOTO2[66].next.push_back(-49);
	GOTO2[67].tg.push_back("DIVIDE");
	GOTO2[67].tg.push_back("DOT");
	GOTO2[67].tg.push_back("EQUAL");
	GOTO2[67].tg.push_back("LBRACE");
	GOTO2[67].tg.push_back("MINUS");
	GOTO2[67].tg.push_back("PLUS");
	GOTO2[67].tg.push_back("TIMES");
	GOTO2[67].tg.push_back("SEMICOLON");
	GOTO2[67].next.push_back(96);
	GOTO2[67].next.push_back(97);
	GOTO2[67].next.push_back(98);
	GOTO2[67].next.push_back(99);
	GOTO2[67].next.push_back(100);
	GOTO2[67].next.push_back(101);
	GOTO2[67].next.push_back(102);
	GOTO2[67].next.push_back(-28);
	GOTO2[68].tg.push_back("DIVIDE");
	GOTO2[68].tg.push_back("DOT");
	GOTO2[68].tg.push_back("EQUAL");
	GOTO2[68].tg.push_back("LBRACE");
	GOTO2[68].tg.push_back("MINUS");
	GOTO2[68].tg.push_back("PLUS");
	GOTO2[68].tg.push_back("SEMICOLON");
	GOTO2[68].tg.push_back("TIMES");
	GOTO2[68].next.push_back(-48);
	GOTO2[68].next.push_back(-48);
	GOTO2[68].next.push_back(-48);
	GOTO2[68].next.push_back(-48);
	GOTO2[68].next.push_back(-48);
	GOTO2[68].next.push_back(-48);
	GOTO2[68].next.push_back(-48);
	GOTO2[68].next.push_back(-48);
	GOTO2[69].tg.push_back("SEMICOLON");
	GOTO2[69].next.push_back(-25);
	GOTO2[70].tg.push_back("LPAR");
	GOTO2[70].tg.push_back("MINUS");
	GOTO2[70].tg.push_back("NAME");
	GOTO2[70].tg.push_back("NUMBER");
	GOTO2[70].tg.push_back("var");
	GOTO2[70].tg.push_back("exp");
	GOTO2[70].next.push_back(63);
	GOTO2[70].next.push_back(64);
	GOTO2[70].next.push_back(65);
	GOTO2[70].next.push_back(66);
	GOTO2[70].next.push_back(68);
	GOTO2[70].next.push_back(103);
	GOTO2[71].tg.push_back("NAME");
	GOTO2[71].next.push_back(104);
	GOTO2[72].tg.push_back("LPAR");
	GOTO2[72].tg.push_back("MINUS");
	GOTO2[72].tg.push_back("NAME");
	GOTO2[72].tg.push_back("NUMBER");
	GOTO2[72].tg.push_back("exp");
	GOTO2[72].tg.push_back("var");
	GOTO2[72].next.push_back(105);
	GOTO2[72].next.push_back(106);
	GOTO2[72].next.push_back(107);
	GOTO2[72].next.push_back(108);
	GOTO2[72].next.push_back(109);
	GOTO2[72].next.push_back(110);
	GOTO2[73].tg.push_back("C");
	GOTO2[73].tg.push_back("IF");
	GOTO2[73].tg.push_back("LBRACE");
	GOTO2[73].tg.push_back("NAME");
	GOTO2[73].tg.push_back("RETURN");
	GOTO2[73].tg.push_back("T");
	GOTO2[73].tg.push_back("block");
	GOTO2[73].tg.push_back("epsilon");
	GOTO2[73].tg.push_back("lexp");
	GOTO2[73].tg.push_back("statement");
	GOTO2[73].tg.push_back("var");
	GOTO2[73].tg.push_back("statements");
	GOTO2[73].next.push_back(42);
	GOTO2[73].next.push_back(43);
	GOTO2[73].next.push_back(44);
	GOTO2[73].next.push_back(45);
	GOTO2[73].next.push_back(46);
	GOTO2[73].next.push_back(47);
	GOTO2[73].next.push_back(48);
	GOTO2[73].next.push_back(49);
	GOTO2[73].next.push_back(50);
	GOTO2[73].next.push_back(51);
	GOTO2[73].next.push_back(53);
	GOTO2[73].next.push_back(111);
	GOTO2[74].tg.push_back("FLOAT");
	GOTO2[74].tg.push_back("INT");
	GOTO2[74].tg.push_back("STRUCT");
	GOTO2[74].tg.push_back("epsilon");
	GOTO2[74].next.push_back(-12);
	GOTO2[74].next.push_back(-12);
	GOTO2[74].next.push_back(-12);
	GOTO2[74].next.push_back(-12);
	GOTO2[75].tg.push_back("ELSE");
	GOTO2[75].tg.push_back("SEMICOLON");
	GOTO2[75].next.push_back(85);
	GOTO2[75].next.push_back(-26);
	GOTO2[76].tg.push_back("C");
	GOTO2[76].tg.push_back("IF");
	GOTO2[76].tg.push_back("LBRACE");
	GOTO2[76].tg.push_back("NAME");
	GOTO2[76].tg.push_back("RETURN");
	GOTO2[76].tg.push_back("T");
	GOTO2[76].tg.push_back("block");
	GOTO2[76].tg.push_back("epsilon");
	GOTO2[76].tg.push_back("lexp");
	GOTO2[76].tg.push_back("statement");
	GOTO2[76].tg.push_back("var");
	GOTO2[76].tg.push_back("statements");
	GOTO2[76].next.push_back(42);
	GOTO2[76].next.push_back(43);
	GOTO2[76].next.push_back(44);
	GOTO2[76].next.push_back(45);
	GOTO2[76].next.push_back(46);
	GOTO2[76].next.push_back(47);
	GOTO2[76].next.push_back(48);
	GOTO2[76].next.push_back(49);
	GOTO2[76].next.push_back(50);
	GOTO2[76].next.push_back(51);
	GOTO2[76].next.push_back(53);
	GOTO2[76].next.push_back(112);
	GOTO2[77].tg.push_back("LPAR");
	GOTO2[77].tg.push_back("MINUS");
	GOTO2[77].tg.push_back("NAME");
	GOTO2[77].tg.push_back("NUMBER");
	GOTO2[77].tg.push_back("var");
	GOTO2[77].tg.push_back("exp");
	GOTO2[77].next.push_back(86);
	GOTO2[77].next.push_back(87);
	GOTO2[77].next.push_back(88);
	GOTO2[77].next.push_back(89);
	GOTO2[77].next.push_back(91);
	GOTO2[77].next.push_back(113);
	GOTO2[78].tg.push_back("LPAR");
	GOTO2[78].tg.push_back("MINUS");
	GOTO2[78].tg.push_back("NAME");
	GOTO2[78].tg.push_back("NUMBER");
	GOTO2[78].tg.push_back("var");
	GOTO2[78].tg.push_back("exp");
	GOTO2[78].next.push_back(77);
	GOTO2[78].next.push_back(78);
	GOTO2[78].next.push_back(79);
	GOTO2[78].next.push_back(80);
	GOTO2[78].next.push_back(82);
	GOTO2[78].next.push_back(114);
	GOTO2[79].tg.push_back("LPAR");
	GOTO2[79].tg.push_back("DIVIDE");
	GOTO2[79].tg.push_back("DOT");
	GOTO2[79].tg.push_back("ELSE");
	GOTO2[79].tg.push_back("EQUAL");
	GOTO2[79].tg.push_back("LBRACE");
	GOTO2[79].tg.push_back("MINUS");
	GOTO2[79].tg.push_back("PLUS");
	GOTO2[79].tg.push_back("SEMICOLON");
	GOTO2[79].tg.push_back("TIMES");
	GOTO2[79].next.push_back(115);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[79].next.push_back(-54);
	GOTO2[80].tg.push_back("DIVIDE");
	GOTO2[80].tg.push_back("DOT");
	GOTO2[80].tg.push_back("ELSE");
	GOTO2[80].tg.push_back("EQUAL");
	GOTO2[80].tg.push_back("LBRACE");
	GOTO2[80].tg.push_back("MINUS");
	GOTO2[80].tg.push_back("PLUS");
	GOTO2[80].tg.push_back("SEMICOLON");
	GOTO2[80].tg.push_back("TIMES");
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[80].next.push_back(-49);
	GOTO2[81].tg.push_back("DIVIDE");
	GOTO2[81].tg.push_back("DOT");
	GOTO2[81].tg.push_back("EQUAL");
	GOTO2[81].tg.push_back("LBRACE");
	GOTO2[81].tg.push_back("MINUS");
	GOTO2[81].tg.push_back("PLUS");
	GOTO2[81].tg.push_back("TIMES");
	GOTO2[81].tg.push_back("ELSE");
	GOTO2[81].tg.push_back("SEMICOLON");
	GOTO2[81].next.push_back(116);
	GOTO2[81].next.push_back(117);
	GOTO2[81].next.push_back(118);
	GOTO2[81].next.push_back(119);
	GOTO2[81].next.push_back(120);
	GOTO2[81].next.push_back(121);
	GOTO2[81].next.push_back(122);
	GOTO2[81].next.push_back(-28);
	GOTO2[81].next.push_back(-28);
	GOTO2[82].tg.push_back("DIVIDE");
	GOTO2[82].tg.push_back("DOT");
	GOTO2[82].tg.push_back("ELSE");
	GOTO2[82].tg.push_back("EQUAL");
	GOTO2[82].tg.push_back("LBRACE");
	GOTO2[82].tg.push_back("MINUS");
	GOTO2[82].tg.push_back("PLUS");
	GOTO2[82].tg.push_back("SEMICOLON");
	GOTO2[82].tg.push_back("TIMES");
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[82].next.push_back(-48);
	GOTO2[83].tg.push_back("ELSE");
	GOTO2[83].tg.push_back("SEMICOLON");
	GOTO2[83].next.push_back(-25);
	GOTO2[83].next.push_back(-25);
	GOTO2[84].tg.push_back("LPAR");
	GOTO2[84].tg.push_back("MINUS");
	GOTO2[84].tg.push_back("NAME");
	GOTO2[84].tg.push_back("NUMBER");
	GOTO2[84].tg.push_back("var");
	GOTO2[84].tg.push_back("exp");
	GOTO2[84].next.push_back(77);
	GOTO2[84].next.push_back(78);
	GOTO2[84].next.push_back(79);
	GOTO2[84].next.push_back(80);
	GOTO2[84].next.push_back(82);
	GOTO2[84].next.push_back(123);
	GOTO2[85].tg.push_back("IF");
	GOTO2[85].tg.push_back("LBRACE");
	GOTO2[85].tg.push_back("NAME");
	GOTO2[85].tg.push_back("RETURN");
	GOTO2[85].next.push_back(-31);
	GOTO2[85].next.push_back(-31);
	GOTO2[85].next.push_back(-31);
	GOTO2[85].next.push_back(-31);
	GOTO2[86].tg.push_back("LPAR");
	GOTO2[86].tg.push_back("MINUS");
	GOTO2[86].tg.push_back("NAME");
	GOTO2[86].tg.push_back("NUMBER");
	GOTO2[86].tg.push_back("var");
	GOTO2[86].tg.push_back("exp");
	GOTO2[86].next.push_back(86);
	GOTO2[86].next.push_back(87);
	GOTO2[86].next.push_back(88);
	GOTO2[86].next.push_back(89);
	GOTO2[86].next.push_back(91);
	GOTO2[86].next.push_back(124);
	GOTO2[87].tg.push_back("LPAR");
	GOTO2[87].tg.push_back("MINUS");
	GOTO2[87].tg.push_back("NAME");
	GOTO2[87].tg.push_back("NUMBER");
	GOTO2[87].tg.push_back("var");
	GOTO2[87].tg.push_back("exp");
	GOTO2[87].next.push_back(86);
	GOTO2[87].next.push_back(87);
	GOTO2[87].next.push_back(88);
	GOTO2[87].next.push_back(89);
	GOTO2[87].next.push_back(91);
	GOTO2[87].next.push_back(125);
	GOTO2[88].tg.push_back("LPAR");
	GOTO2[88].tg.push_back("DIVIDE");
	GOTO2[88].tg.push_back("DOT");
	GOTO2[88].tg.push_back("EQUAL");
	GOTO2[88].tg.push_back("LBRACE");
	GOTO2[88].tg.push_back("MINUS");
	GOTO2[88].tg.push_back("PLUS");
	GOTO2[88].tg.push_back("RPAR");
	GOTO2[88].tg.push_back("TIMES");
	GOTO2[88].next.push_back(126);
	GOTO2[88].next.push_back(-54);
	GOTO2[88].next.push_back(-54);
	GOTO2[88].next.push_back(-54);
	GOTO2[88].next.push_back(-54);
	GOTO2[88].next.push_back(-54);
	GOTO2[88].next.push_back(-54);
	GOTO2[88].next.push_back(-54);
	GOTO2[88].next.push_back(-54);
	GOTO2[89].tg.push_back("DIVIDE");
	GOTO2[89].tg.push_back("DOT");
	GOTO2[89].tg.push_back("EQUAL");
	GOTO2[89].tg.push_back("LBRACE");
	GOTO2[89].tg.push_back("MINUS");
	GOTO2[89].tg.push_back("PLUS");
	GOTO2[89].tg.push_back("RPAR");
	GOTO2[89].tg.push_back("TIMES");
	GOTO2[89].next.push_back(-49);
	GOTO2[89].next.push_back(-49);
	GOTO2[89].next.push_back(-49);
	GOTO2[89].next.push_back(-49);
	GOTO2[89].next.push_back(-49);
	GOTO2[89].next.push_back(-49);
	GOTO2[89].next.push_back(-49);
	GOTO2[89].next.push_back(-49);
	GOTO2[90].tg.push_back("DIVIDE");
	GOTO2[90].tg.push_back("DOT");
	GOTO2[90].tg.push_back("EQUAL");
	GOTO2[90].tg.push_back("LBRACE");
	GOTO2[90].tg.push_back("MINUS");
	GOTO2[90].tg.push_back("PLUS");
	GOTO2[90].tg.push_back("RPAR");
	GOTO2[90].tg.push_back("TIMES");
	GOTO2[90].next.push_back(127);
	GOTO2[90].next.push_back(128);
	GOTO2[90].next.push_back(129);
	GOTO2[90].next.push_back(130);
	GOTO2[90].next.push_back(131);
	GOTO2[90].next.push_back(132);
	GOTO2[90].next.push_back(133);
	GOTO2[90].next.push_back(134);
	GOTO2[91].tg.push_back("DIVIDE");
	GOTO2[91].tg.push_back("DOT");
	GOTO2[91].tg.push_back("EQUAL");
	GOTO2[91].tg.push_back("LBRACE");
	GOTO2[91].tg.push_back("MINUS");
	GOTO2[91].tg.push_back("PLUS");
	GOTO2[91].tg.push_back("RPAR");
	GOTO2[91].tg.push_back("TIMES");
	GOTO2[91].next.push_back(-48);
	GOTO2[91].next.push_back(-48);
	GOTO2[91].next.push_back(-48);
	GOTO2[91].next.push_back(-48);
	GOTO2[91].next.push_back(-48);
	GOTO2[91].next.push_back(-48);
	GOTO2[91].next.push_back(-48);
	GOTO2[91].next.push_back(-48);
	GOTO2[92].tg.push_back("RBRACE");
	GOTO2[92].next.push_back(135);
	GOTO2[93].tg.push_back("DIVIDE");
	GOTO2[93].tg.push_back("DOT");
	GOTO2[93].tg.push_back("EQUAL");
	GOTO2[93].tg.push_back("LBRACE");
	GOTO2[93].tg.push_back("MINUS");
	GOTO2[93].tg.push_back("PLUS");
	GOTO2[93].tg.push_back("TIMES");
	GOTO2[93].tg.push_back("RPAR");
	GOTO2[93].next.push_back(127);
	GOTO2[93].next.push_back(128);
	GOTO2[93].next.push_back(129);
	GOTO2[93].next.push_back(130);
	GOTO2[93].next.push_back(131);
	GOTO2[93].next.push_back(132);
	GOTO2[93].next.push_back(134);
	GOTO2[93].next.push_back(136);
	GOTO2[94].tg.push_back("DIVIDE");
	GOTO2[94].tg.push_back("DOT");
	GOTO2[94].tg.push_back("EQUAL");
	GOTO2[94].tg.push_back("LBRACE");
	GOTO2[94].tg.push_back("MINUS");
	GOTO2[94].tg.push_back("PLUS");
	GOTO2[94].tg.push_back("TIMES");
	GOTO2[94].tg.push_back("SEMICOLON");
	GOTO2[94].next.push_back(96);
	GOTO2[94].next.push_back(97);
	GOTO2[94].next.push_back(98);
	GOTO2[94].next.push_back(99);
	GOTO2[94].next.push_back(100);
	GOTO2[94].next.push_back(101);
	GOTO2[94].next.push_back(102);
	GOTO2[94].next.push_back(-47);
	GOTO2[95].tg.push_back("LPAR");
	GOTO2[95].tg.push_back("MINUS");
	GOTO2[95].tg.push_back("NAME");
	GOTO2[95].tg.push_back("NUMBER");
	GOTO2[95].tg.push_back("RPAR");
	GOTO2[95].tg.push_back("exp");
	GOTO2[95].tg.push_back("exps");
	GOTO2[95].tg.push_back("var");
	GOTO2[95].next.push_back(137);
	GOTO2[95].next.push_back(138);
	GOTO2[95].next.push_back(139);
	GOTO2[95].next.push_back(140);
	GOTO2[95].next.push_back(141);
	GOTO2[95].next.push_back(142);
	GOTO2[95].next.push_back(143);
	GOTO2[95].next.push_back(144);
	GOTO2[96].tg.push_back("LPAR");
	GOTO2[96].tg.push_back("MINUS");
	GOTO2[96].tg.push_back("NAME");
	GOTO2[96].tg.push_back("NUMBER");
	GOTO2[96].tg.push_back("var");
	GOTO2[96].tg.push_back("exp");
	GOTO2[96].next.push_back(63);
	GOTO2[96].next.push_back(64);
	GOTO2[96].next.push_back(65);
	GOTO2[96].next.push_back(66);
	GOTO2[96].next.push_back(68);
	GOTO2[96].next.push_back(145);
	GOTO2[97].tg.push_back("NAME");
	GOTO2[97].next.push_back(146);
	GOTO2[98].tg.push_back("LPAR");
	GOTO2[98].tg.push_back("MINUS");
	GOTO2[98].tg.push_back("NAME");
	GOTO2[98].tg.push_back("NUMBER");
	GOTO2[98].tg.push_back("var");
	GOTO2[98].tg.push_back("exp");
	GOTO2[98].next.push_back(63);
	GOTO2[98].next.push_back(64);
	GOTO2[98].next.push_back(65);
	GOTO2[98].next.push_back(66);
	GOTO2[98].next.push_back(68);
	GOTO2[98].next.push_back(147);
	GOTO2[99].tg.push_back("LPAR");
	GOTO2[99].tg.push_back("MINUS");
	GOTO2[99].tg.push_back("NAME");
	GOTO2[99].tg.push_back("NUMBER");
	GOTO2[99].tg.push_back("var");
	GOTO2[99].tg.push_back("exp");
	GOTO2[99].next.push_back(105);
	GOTO2[99].next.push_back(106);
	GOTO2[99].next.push_back(107);
	GOTO2[99].next.push_back(108);
	GOTO2[99].next.push_back(110);
	GOTO2[99].next.push_back(148);
	GOTO2[100].tg.push_back("LPAR");
	GOTO2[100].tg.push_back("MINUS");
	GOTO2[100].tg.push_back("NAME");
	GOTO2[100].tg.push_back("NUMBER");
	GOTO2[100].tg.push_back("var");
	GOTO2[100].tg.push_back("exp");
	GOTO2[100].next.push_back(63);
	GOTO2[100].next.push_back(64);
	GOTO2[100].next.push_back(65);
	GOTO2[100].next.push_back(66);
	GOTO2[100].next.push_back(68);
	GOTO2[100].next.push_back(149);
	GOTO2[101].tg.push_back("LPAR");
	GOTO2[101].tg.push_back("MINUS");
	GOTO2[101].tg.push_back("NAME");
	GOTO2[101].tg.push_back("NUMBER");
	GOTO2[101].tg.push_back("var");
	GOTO2[101].tg.push_back("exp");
	GOTO2[101].next.push_back(63);
	GOTO2[101].next.push_back(64);
	GOTO2[101].next.push_back(65);
	GOTO2[101].next.push_back(66);
	GOTO2[101].next.push_back(68);
	GOTO2[101].next.push_back(150);
	GOTO2[102].tg.push_back("LPAR");
	GOTO2[102].tg.push_back("MINUS");
	GOTO2[102].tg.push_back("NAME");
	GOTO2[102].tg.push_back("NUMBER");
	GOTO2[102].tg.push_back("var");
	GOTO2[102].tg.push_back("exp");
	GOTO2[102].next.push_back(63);
	GOTO2[102].next.push_back(64);
	GOTO2[102].next.push_back(65);
	GOTO2[102].next.push_back(66);
	GOTO2[102].next.push_back(68);
	GOTO2[102].next.push_back(151);
	GOTO2[103].tg.push_back("DIVIDE");
	GOTO2[103].tg.push_back("DOT");
	GOTO2[103].tg.push_back("EQUAL");
	GOTO2[103].tg.push_back("LBRACE");
	GOTO2[103].tg.push_back("MINUS");
	GOTO2[103].tg.push_back("PLUS");
	GOTO2[103].tg.push_back("TIMES");
	GOTO2[103].tg.push_back("SEMICOLON");
	GOTO2[103].next.push_back(96);
	GOTO2[103].next.push_back(97);
	GOTO2[103].next.push_back(98);
	GOTO2[103].next.push_back(99);
	GOTO2[103].next.push_back(100);
	GOTO2[103].next.push_back(101);
	GOTO2[103].next.push_back(102);
	GOTO2[103].next.push_back(-27);
	GOTO2[104].tg.push_back("ASSIGN");
	GOTO2[104].tg.push_back("DOT");
	GOTO2[104].tg.push_back("LBRACE");
	GOTO2[104].next.push_back(-35);
	GOTO2[104].next.push_back(-35);
	GOTO2[104].next.push_back(-35);
	GOTO2[105].tg.push_back("LPAR");
	GOTO2[105].tg.push_back("MINUS");
	GOTO2[105].tg.push_back("NAME");
	GOTO2[105].tg.push_back("NUMBER");
	GOTO2[105].tg.push_back("var");
	GOTO2[105].tg.push_back("exp");
	GOTO2[105].next.push_back(86);
	GOTO2[105].next.push_back(87);
	GOTO2[105].next.push_back(88);
	GOTO2[105].next.push_back(89);
	GOTO2[105].next.push_back(91);
	GOTO2[105].next.push_back(152);
	GOTO2[106].tg.push_back("LPAR");
	GOTO2[106].tg.push_back("MINUS");
	GOTO2[106].tg.push_back("NAME");
	GOTO2[106].tg.push_back("NUMBER");
	GOTO2[106].tg.push_back("var");
	GOTO2[106].tg.push_back("exp");
	GOTO2[106].next.push_back(105);
	GOTO2[106].next.push_back(106);
	GOTO2[106].next.push_back(107);
	GOTO2[106].next.push_back(108);
	GOTO2[106].next.push_back(110);
	GOTO2[106].next.push_back(153);
	GOTO2[107].tg.push_back("LPAR");
	GOTO2[107].tg.push_back("DIVIDE");
	GOTO2[107].tg.push_back("DOT");
	GOTO2[107].tg.push_back("EQUAL");
	GOTO2[107].tg.push_back("LBRACE");
	GOTO2[107].tg.push_back("MINUS");
	GOTO2[107].tg.push_back("PLUS");
	GOTO2[107].tg.push_back("RBRACE");
	GOTO2[107].tg.push_back("TIMES");
	GOTO2[107].next.push_back(154);
	GOTO2[107].next.push_back(-54);
	GOTO2[107].next.push_back(-54);
	GOTO2[107].next.push_back(-54);
	GOTO2[107].next.push_back(-54);
	GOTO2[107].next.push_back(-54);
	GOTO2[107].next.push_back(-54);
	GOTO2[107].next.push_back(-54);
	GOTO2[107].next.push_back(-54);
	GOTO2[108].tg.push_back("DIVIDE");
	GOTO2[108].tg.push_back("DOT");
	GOTO2[108].tg.push_back("EQUAL");
	GOTO2[108].tg.push_back("LBRACE");
	GOTO2[108].tg.push_back("MINUS");
	GOTO2[108].tg.push_back("PLUS");
	GOTO2[108].tg.push_back("RBRACE");
	GOTO2[108].tg.push_back("TIMES");
	GOTO2[108].next.push_back(-49);
	GOTO2[108].next.push_back(-49);
	GOTO2[108].next.push_back(-49);
	GOTO2[108].next.push_back(-49);
	GOTO2[108].next.push_back(-49);
	GOTO2[108].next.push_back(-49);
	GOTO2[108].next.push_back(-49);
	GOTO2[108].next.push_back(-49);
	GOTO2[109].tg.push_back("DIVIDE");
	GOTO2[109].tg.push_back("DOT");
	GOTO2[109].tg.push_back("EQUAL");
	GOTO2[109].tg.push_back("LBRACE");
	GOTO2[109].tg.push_back("MINUS");
	GOTO2[109].tg.push_back("PLUS");
	GOTO2[109].tg.push_back("RBRACE");
	GOTO2[109].tg.push_back("TIMES");
	GOTO2[109].next.push_back(155);
	GOTO2[109].next.push_back(156);
	GOTO2[109].next.push_back(157);
	GOTO2[109].next.push_back(158);
	GOTO2[109].next.push_back(159);
	GOTO2[109].next.push_back(160);
	GOTO2[109].next.push_back(161);
	GOTO2[109].next.push_back(162);
	GOTO2[110].tg.push_back("DIVIDE");
	GOTO2[110].tg.push_back("DOT");
	GOTO2[110].tg.push_back("EQUAL");
	GOTO2[110].tg.push_back("LBRACE");
	GOTO2[110].tg.push_back("MINUS");
	GOTO2[110].tg.push_back("PLUS");
	GOTO2[110].tg.push_back("RBRACE");
	GOTO2[110].tg.push_back("TIMES");
	GOTO2[110].next.push_back(-48);
	GOTO2[110].next.push_back(-48);
	GOTO2[110].next.push_back(-48);
	GOTO2[110].next.push_back(-48);
	GOTO2[110].next.push_back(-48);
	GOTO2[110].next.push_back(-48);
	GOTO2[110].next.push_back(-48);
	GOTO2[110].next.push_back(-48);
	GOTO2[111].tg.push_back("RBRACE");
	GOTO2[111].next.push_back(-23);
	GOTO2[112].tg.push_back("RBRACE");
	GOTO2[112].next.push_back(163);
	GOTO2[113].tg.push_back("DIVIDE");
	GOTO2[113].tg.push_back("DOT");
	GOTO2[113].tg.push_back("EQUAL");
	GOTO2[113].tg.push_back("LBRACE");
	GOTO2[113].tg.push_back("MINUS");
	GOTO2[113].tg.push_back("PLUS");
	GOTO2[113].tg.push_back("TIMES");
	GOTO2[113].tg.push_back("RPAR");
	GOTO2[113].next.push_back(127);
	GOTO2[113].next.push_back(128);
	GOTO2[113].next.push_back(129);
	GOTO2[113].next.push_back(130);
	GOTO2[113].next.push_back(131);
	GOTO2[113].next.push_back(132);
	GOTO2[113].next.push_back(134);
	GOTO2[113].next.push_back(164);
	GOTO2[114].tg.push_back("DIVIDE");
	GOTO2[114].tg.push_back("DOT");
	GOTO2[114].tg.push_back("EQUAL");
	GOTO2[114].tg.push_back("LBRACE");
	GOTO2[114].tg.push_back("MINUS");
	GOTO2[114].tg.push_back("PLUS");
	GOTO2[114].tg.push_back("TIMES");
	GOTO2[114].tg.push_back("ELSE");
	GOTO2[114].tg.push_back("SEMICOLON");
	GOTO2[114].next.push_back(116);
	GOTO2[114].next.push_back(117);
	GOTO2[114].next.push_back(118);
	GOTO2[114].next.push_back(119);
	GOTO2[114].next.push_back(120);
	GOTO2[114].next.push_back(121);
	GOTO2[114].next.push_back(122);
	GOTO2[114].next.push_back(-47);
	GOTO2[114].next.push_back(-47);
	GOTO2[115].tg.push_back("LPAR");
	GOTO2[115].tg.push_back("MINUS");
	GOTO2[115].tg.push_back("NAME");
	GOTO2[115].tg.push_back("NUMBER");
	GOTO2[115].tg.push_back("exp");
	GOTO2[115].tg.push_back("var");
	GOTO2[115].tg.push_back("RPAR");
	GOTO2[115].tg.push_back("exps");
	GOTO2[115].next.push_back(137);
	GOTO2[115].next.push_back(138);
	GOTO2[115].next.push_back(139);
	GOTO2[115].next.push_back(140);
	GOTO2[115].next.push_back(142);
	GOTO2[115].next.push_back(144);
	GOTO2[115].next.push_back(165);
	GOTO2[115].next.push_back(166);
	GOTO2[116].tg.push_back("LPAR");
	GOTO2[116].tg.push_back("MINUS");
	GOTO2[116].tg.push_back("NAME");
	GOTO2[116].tg.push_back("NUMBER");
	GOTO2[116].tg.push_back("var");
	GOTO2[116].tg.push_back("exp");
	GOTO2[116].next.push_back(77);
	GOTO2[116].next.push_back(78);
	GOTO2[116].next.push_back(79);
	GOTO2[116].next.push_back(80);
	GOTO2[116].next.push_back(82);
	GOTO2[116].next.push_back(167);
	GOTO2[117].tg.push_back("NAME");
	GOTO2[117].next.push_back(168);
	GOTO2[118].tg.push_back("LPAR");
	GOTO2[118].tg.push_back("MINUS");
	GOTO2[118].tg.push_back("NAME");
	GOTO2[118].tg.push_back("NUMBER");
	GOTO2[118].tg.push_back("var");
	GOTO2[118].tg.push_back("exp");
	GOTO2[118].next.push_back(77);
	GOTO2[118].next.push_back(78);
	GOTO2[118].next.push_back(79);
	GOTO2[118].next.push_back(80);
	GOTO2[118].next.push_back(82);
	GOTO2[118].next.push_back(169);
	GOTO2[119].tg.push_back("LPAR");
	GOTO2[119].tg.push_back("MINUS");
	GOTO2[119].tg.push_back("NAME");
	GOTO2[119].tg.push_back("NUMBER");
	GOTO2[119].tg.push_back("var");
	GOTO2[119].tg.push_back("exp");
	GOTO2[119].next.push_back(105);
	GOTO2[119].next.push_back(106);
	GOTO2[119].next.push_back(107);
	GOTO2[119].next.push_back(108);
	GOTO2[119].next.push_back(110);
	GOTO2[119].next.push_back(170);
	GOTO2[120].tg.push_back("LPAR");
	GOTO2[120].tg.push_back("MINUS");
	GOTO2[120].tg.push_back("NAME");
	GOTO2[120].tg.push_back("NUMBER");
	GOTO2[120].tg.push_back("var");
	GOTO2[120].tg.push_back("exp");
	GOTO2[120].next.push_back(77);
	GOTO2[120].next.push_back(78);
	GOTO2[120].next.push_back(79);
	GOTO2[120].next.push_back(80);
	GOTO2[120].next.push_back(82);
	GOTO2[120].next.push_back(171);
	GOTO2[121].tg.push_back("LPAR");
	GOTO2[121].tg.push_back("MINUS");
	GOTO2[121].tg.push_back("NAME");
	GOTO2[121].tg.push_back("NUMBER");
	GOTO2[121].tg.push_back("var");
	GOTO2[121].tg.push_back("exp");
	GOTO2[121].next.push_back(77);
	GOTO2[121].next.push_back(78);
	GOTO2[121].next.push_back(79);
	GOTO2[121].next.push_back(80);
	GOTO2[121].next.push_back(82);
	GOTO2[121].next.push_back(172);
	GOTO2[122].tg.push_back("LPAR");
	GOTO2[122].tg.push_back("MINUS");
	GOTO2[122].tg.push_back("NAME");
	GOTO2[122].tg.push_back("NUMBER");
	GOTO2[122].tg.push_back("var");
	GOTO2[122].tg.push_back("exp");
	GOTO2[122].next.push_back(77);
	GOTO2[122].next.push_back(78);
	GOTO2[122].next.push_back(79);
	GOTO2[122].next.push_back(80);
	GOTO2[122].next.push_back(82);
	GOTO2[122].next.push_back(173);
	GOTO2[123].tg.push_back("DIVIDE");
	GOTO2[123].tg.push_back("DOT");
	GOTO2[123].tg.push_back("EQUAL");
	GOTO2[123].tg.push_back("LBRACE");
	GOTO2[123].tg.push_back("MINUS");
	GOTO2[123].tg.push_back("PLUS");
	GOTO2[123].tg.push_back("TIMES");
	GOTO2[123].tg.push_back("ELSE");
	GOTO2[123].tg.push_back("SEMICOLON");
	GOTO2[123].next.push_back(116);
	GOTO2[123].next.push_back(117);
	GOTO2[123].next.push_back(118);
	GOTO2[123].next.push_back(119);
	GOTO2[123].next.push_back(120);
	GOTO2[123].next.push_back(121);
	GOTO2[123].next.push_back(122);
	GOTO2[123].next.push_back(-27);
	GOTO2[123].next.push_back(-27);
	GOTO2[124].tg.push_back("DIVIDE");
	GOTO2[124].tg.push_back("DOT");
	GOTO2[124].tg.push_back("EQUAL");
	GOTO2[124].tg.push_back("LBRACE");
	GOTO2[124].tg.push_back("MINUS");
	GOTO2[124].tg.push_back("PLUS");
	GOTO2[124].tg.push_back("TIMES");
	GOTO2[124].tg.push_back("RPAR");
	GOTO2[124].next.push_back(127);
	GOTO2[124].next.push_back(128);
	GOTO2[124].next.push_back(129);
	GOTO2[124].next.push_back(130);
	GOTO2[124].next.push_back(131);
	GOTO2[124].next.push_back(132);
	GOTO2[124].next.push_back(134);
	GOTO2[124].next.push_back(174);
	GOTO2[125].tg.push_back("DIVIDE");
	GOTO2[125].tg.push_back("DOT");
	GOTO2[125].tg.push_back("EQUAL");
	GOTO2[125].tg.push_back("LBRACE");
	GOTO2[125].tg.push_back("MINUS");
	GOTO2[125].tg.push_back("PLUS");
	GOTO2[125].tg.push_back("TIMES");
	GOTO2[125].tg.push_back("RPAR");
	GOTO2[125].next.push_back(127);
	GOTO2[125].next.push_back(128);
	GOTO2[125].next.push_back(129);
	GOTO2[125].next.push_back(130);
	GOTO2[125].next.push_back(131);
	GOTO2[125].next.push_back(132);
	GOTO2[125].next.push_back(134);
	GOTO2[125].next.push_back(-47);
	GOTO2[126].tg.push_back("LPAR");
	GOTO2[126].tg.push_back("MINUS");
	GOTO2[126].tg.push_back("NAME");
	GOTO2[126].tg.push_back("NUMBER");
	GOTO2[126].tg.push_back("exp");
	GOTO2[126].tg.push_back("var");
	GOTO2[126].tg.push_back("RPAR");
	GOTO2[126].tg.push_back("exps");
	GOTO2[126].next.push_back(137);
	GOTO2[126].next.push_back(138);
	GOTO2[126].next.push_back(139);
	GOTO2[126].next.push_back(140);
	GOTO2[126].next.push_back(142);
	GOTO2[126].next.push_back(144);
	GOTO2[126].next.push_back(175);
	GOTO2[126].next.push_back(176);
	GOTO2[127].tg.push_back("LPAR");
	GOTO2[127].tg.push_back("MINUS");
	GOTO2[127].tg.push_back("NAME");
	GOTO2[127].tg.push_back("NUMBER");
	GOTO2[127].tg.push_back("var");
	GOTO2[127].tg.push_back("exp");
	GOTO2[127].next.push_back(86);
	GOTO2[127].next.push_back(87);
	GOTO2[127].next.push_back(88);
	GOTO2[127].next.push_back(89);
	GOTO2[127].next.push_back(91);
	GOTO2[127].next.push_back(177);
	GOTO2[128].tg.push_back("NAME");
	GOTO2[128].next.push_back(178);
	GOTO2[129].tg.push_back("LPAR");
	GOTO2[129].tg.push_back("MINUS");
	GOTO2[129].tg.push_back("NAME");
	GOTO2[129].tg.push_back("NUMBER");
	GOTO2[129].tg.push_back("var");
	GOTO2[129].tg.push_back("exp");
	GOTO2[129].next.push_back(86);
	GOTO2[129].next.push_back(87);
	GOTO2[129].next.push_back(88);
	GOTO2[129].next.push_back(89);
	GOTO2[129].next.push_back(91);
	GOTO2[129].next.push_back(179);
	GOTO2[130].tg.push_back("LPAR");
	GOTO2[130].tg.push_back("MINUS");
	GOTO2[130].tg.push_back("NAME");
	GOTO2[130].tg.push_back("NUMBER");
	GOTO2[130].tg.push_back("var");
	GOTO2[130].tg.push_back("exp");
	GOTO2[130].next.push_back(105);
	GOTO2[130].next.push_back(106);
	GOTO2[130].next.push_back(107);
	GOTO2[130].next.push_back(108);
	GOTO2[130].next.push_back(110);
	GOTO2[130].next.push_back(180);
	GOTO2[131].tg.push_back("LPAR");
	GOTO2[131].tg.push_back("MINUS");
	GOTO2[131].tg.push_back("NAME");
	GOTO2[131].tg.push_back("NUMBER");
	GOTO2[131].tg.push_back("var");
	GOTO2[131].tg.push_back("exp");
	GOTO2[131].next.push_back(86);
	GOTO2[131].next.push_back(87);
	GOTO2[131].next.push_back(88);
	GOTO2[131].next.push_back(89);
	GOTO2[131].next.push_back(91);
	GOTO2[131].next.push_back(181);
	GOTO2[132].tg.push_back("LPAR");
	GOTO2[132].tg.push_back("MINUS");
	GOTO2[132].tg.push_back("NAME");
	GOTO2[132].tg.push_back("NUMBER");
	GOTO2[132].tg.push_back("var");
	GOTO2[132].tg.push_back("exp");
	GOTO2[132].next.push_back(86);
	GOTO2[132].next.push_back(87);
	GOTO2[132].next.push_back(88);
	GOTO2[132].next.push_back(89);
	GOTO2[132].next.push_back(91);
	GOTO2[132].next.push_back(182);
	GOTO2[133].tg.push_back("IF");
	GOTO2[133].tg.push_back("LBRACE");
	GOTO2[133].tg.push_back("NAME");
	GOTO2[133].tg.push_back("RETURN");
	GOTO2[133].next.push_back(-32);
	GOTO2[133].next.push_back(-32);
	GOTO2[133].next.push_back(-32);
	GOTO2[133].next.push_back(-32);
	GOTO2[134].tg.push_back("LPAR");
	GOTO2[134].tg.push_back("MINUS");
	GOTO2[134].tg.push_back("NAME");
	GOTO2[134].tg.push_back("NUMBER");
	GOTO2[134].tg.push_back("var");
	GOTO2[134].tg.push_back("exp");
	GOTO2[134].next.push_back(86);
	GOTO2[134].next.push_back(87);
	GOTO2[134].next.push_back(88);
	GOTO2[134].next.push_back(89);
	GOTO2[134].next.push_back(91);
	GOTO2[134].next.push_back(183);
	GOTO2[135].tg.push_back("SEMICOLON");
	GOTO2[135].next.push_back(-12);
	GOTO2[136].tg.push_back("DIVIDE");
	GOTO2[136].tg.push_back("DOT");
	GOTO2[136].tg.push_back("EQUAL");
	GOTO2[136].tg.push_back("LBRACE");
	GOTO2[136].tg.push_back("MINUS");
	GOTO2[136].tg.push_back("PLUS");
	GOTO2[136].tg.push_back("SEMICOLON");
	GOTO2[136].tg.push_back("TIMES");
	GOTO2[136].next.push_back(-46);
	GOTO2[136].next.push_back(-46);
	GOTO2[136].next.push_back(-46);
	GOTO2[136].next.push_back(-46);
	GOTO2[136].next.push_back(-46);
	GOTO2[136].next.push_back(-46);
	GOTO2[136].next.push_back(-46);
	GOTO2[136].next.push_back(-46);
	GOTO2[137].tg.push_back("LPAR");
	GOTO2[137].tg.push_back("MINUS");
	GOTO2[137].tg.push_back("NAME");
	GOTO2[137].tg.push_back("NUMBER");
	GOTO2[137].tg.push_back("var");
	GOTO2[137].tg.push_back("exp");
	GOTO2[137].next.push_back(86);
	GOTO2[137].next.push_back(87);
	GOTO2[137].next.push_back(88);
	GOTO2[137].next.push_back(89);
	GOTO2[137].next.push_back(91);
	GOTO2[137].next.push_back(184);
	GOTO2[138].tg.push_back("LPAR");
	GOTO2[138].tg.push_back("MINUS");
	GOTO2[138].tg.push_back("NAME");
	GOTO2[138].tg.push_back("NUMBER");
	GOTO2[138].tg.push_back("var");
	GOTO2[138].tg.push_back("exp");
	GOTO2[138].next.push_back(137);
	GOTO2[138].next.push_back(138);
	GOTO2[138].next.push_back(139);
	GOTO2[138].next.push_back(140);
	GOTO2[138].next.push_back(144);
	GOTO2[138].next.push_back(185);
	GOTO2[139].tg.push_back("LPAR");
	GOTO2[139].tg.push_back("COMMA");
	GOTO2[139].tg.push_back("DIVIDE");
	GOTO2[139].tg.push_back("DOT");
	GOTO2[139].tg.push_back("EQUAL");
	GOTO2[139].tg.push_back("LBRACE");
	GOTO2[139].tg.push_back("MINUS");
	GOTO2[139].tg.push_back("PLUS");
	GOTO2[139].tg.push_back("RPAR");
	GOTO2[139].tg.push_back("TIMES");
	GOTO2[139].next.push_back(186);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[139].next.push_back(-54);
	GOTO2[140].tg.push_back("COMMA");
	GOTO2[140].tg.push_back("DIVIDE");
	GOTO2[140].tg.push_back("DOT");
	GOTO2[140].tg.push_back("EQUAL");
	GOTO2[140].tg.push_back("LBRACE");
	GOTO2[140].tg.push_back("MINUS");
	GOTO2[140].tg.push_back("PLUS");
	GOTO2[140].tg.push_back("RPAR");
	GOTO2[140].tg.push_back("TIMES");
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[140].next.push_back(-49);
	GOTO2[141].tg.push_back("DIVIDE");
	GOTO2[141].tg.push_back("DOT");
	GOTO2[141].tg.push_back("EQUAL");
	GOTO2[141].tg.push_back("LBRACE");
	GOTO2[141].tg.push_back("MINUS");
	GOTO2[141].tg.push_back("PLUS");
	GOTO2[141].tg.push_back("SEMICOLON");
	GOTO2[141].tg.push_back("TIMES");
	GOTO2[141].next.push_back(-50);
	GOTO2[141].next.push_back(-50);
	GOTO2[141].next.push_back(-50);
	GOTO2[141].next.push_back(-50);
	GOTO2[141].next.push_back(-50);
	GOTO2[141].next.push_back(-50);
	GOTO2[141].next.push_back(-50);
	GOTO2[141].next.push_back(-50);
	GOTO2[142].tg.push_back("COMMA");
	GOTO2[142].tg.push_back("DIVIDE");
	GOTO2[142].tg.push_back("DOT");
	GOTO2[142].tg.push_back("EQUAL");
	GOTO2[142].tg.push_back("LBRACE");
	GOTO2[142].tg.push_back("MINUS");
	GOTO2[142].tg.push_back("PLUS");
	GOTO2[142].tg.push_back("TIMES");
	GOTO2[142].tg.push_back("RPAR");
	GOTO2[142].next.push_back(187);
	GOTO2[142].next.push_back(188);
	GOTO2[142].next.push_back(189);
	GOTO2[142].next.push_back(190);
	GOTO2[142].next.push_back(191);
	GOTO2[142].next.push_back(192);
	GOTO2[142].next.push_back(193);
	GOTO2[142].next.push_back(194);
	GOTO2[142].next.push_back(-52);
	GOTO2[143].tg.push_back("RPAR");
	GOTO2[143].next.push_back(195);
	GOTO2[144].tg.push_back("COMMA");
	GOTO2[144].tg.push_back("DIVIDE");
	GOTO2[144].tg.push_back("DOT");
	GOTO2[144].tg.push_back("EQUAL");
	GOTO2[144].tg.push_back("LBRACE");
	GOTO2[144].tg.push_back("MINUS");
	GOTO2[144].tg.push_back("PLUS");
	GOTO2[144].tg.push_back("RPAR");
	GOTO2[144].tg.push_back("TIMES");
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[144].next.push_back(-48);
	GOTO2[145].tg.push_back("DIVIDE");
	GOTO2[145].tg.push_back("DOT");
	GOTO2[145].tg.push_back("EQUAL");
	GOTO2[145].tg.push_back("LBRACE");
	GOTO2[145].tg.push_back("MINUS");
	GOTO2[145].tg.push_back("PLUS");
	GOTO2[145].tg.push_back("TIMES");
	GOTO2[145].tg.push_back("SEMICOLON");
	GOTO2[145].next.push_back(96);
	GOTO2[145].next.push_back(97);
	GOTO2[145].next.push_back(98);
	GOTO2[145].next.push_back(99);
	GOTO2[145].next.push_back(100);
	GOTO2[145].next.push_back(101);
	GOTO2[145].next.push_back(102);
	GOTO2[145].next.push_back(-44);
	GOTO2[146].tg.push_back("DIVIDE");
	GOTO2[146].tg.push_back("DOT");
	GOTO2[146].tg.push_back("EQUAL");
	GOTO2[146].tg.push_back("LBRACE");
	GOTO2[146].tg.push_back("MINUS");
	GOTO2[146].tg.push_back("PLUS");
	GOTO2[146].tg.push_back("SEMICOLON");
	GOTO2[146].tg.push_back("TIMES");
	GOTO2[146].next.push_back(-39);
	GOTO2[146].next.push_back(-39);
	GOTO2[146].next.push_back(-39);
	GOTO2[146].next.push_back(-39);
	GOTO2[146].next.push_back(-39);
	GOTO2[146].next.push_back(-39);
	GOTO2[146].next.push_back(-39);
	GOTO2[146].next.push_back(-39);
	GOTO2[147].tg.push_back("DIVIDE");
	GOTO2[147].tg.push_back("DOT");
	GOTO2[147].tg.push_back("EQUAL");
	GOTO2[147].tg.push_back("LBRACE");
	GOTO2[147].tg.push_back("MINUS");
	GOTO2[147].tg.push_back("PLUS");
	GOTO2[147].tg.push_back("TIMES");
	GOTO2[147].tg.push_back("SEMICOLON");
	GOTO2[147].next.push_back(96);
	GOTO2[147].next.push_back(97);
	GOTO2[147].next.push_back(98);
	GOTO2[147].next.push_back(99);
	GOTO2[147].next.push_back(100);
	GOTO2[147].next.push_back(101);
	GOTO2[147].next.push_back(102);
	GOTO2[147].next.push_back(-45);
	GOTO2[148].tg.push_back("DIVIDE");
	GOTO2[148].tg.push_back("DOT");
	GOTO2[148].tg.push_back("EQUAL");
	GOTO2[148].tg.push_back("LBRACE");
	GOTO2[148].tg.push_back("MINUS");
	GOTO2[148].tg.push_back("PLUS");
	GOTO2[148].tg.push_back("TIMES");
	GOTO2[148].tg.push_back("RBRACE");
	GOTO2[148].next.push_back(155);
	GOTO2[148].next.push_back(156);
	GOTO2[148].next.push_back(157);
	GOTO2[148].next.push_back(158);
	GOTO2[148].next.push_back(159);
	GOTO2[148].next.push_back(160);
	GOTO2[148].next.push_back(162);
	GOTO2[148].next.push_back(196);
	GOTO2[149].tg.push_back("DIVIDE");
	GOTO2[149].tg.push_back("DOT");
	GOTO2[149].tg.push_back("EQUAL");
	GOTO2[149].tg.push_back("LBRACE");
	GOTO2[149].tg.push_back("MINUS");
	GOTO2[149].tg.push_back("PLUS");
	GOTO2[149].tg.push_back("TIMES");
	GOTO2[149].tg.push_back("SEMICOLON");
	GOTO2[149].next.push_back(96);
	GOTO2[149].next.push_back(97);
	GOTO2[149].next.push_back(98);
	GOTO2[149].next.push_back(99);
	GOTO2[149].next.push_back(100);
	GOTO2[149].next.push_back(101);
	GOTO2[149].next.push_back(102);
	GOTO2[149].next.push_back(-42);
	GOTO2[150].tg.push_back("DIVIDE");
	GOTO2[150].tg.push_back("DOT");
	GOTO2[150].tg.push_back("EQUAL");
	GOTO2[150].tg.push_back("LBRACE");
	GOTO2[150].tg.push_back("MINUS");
	GOTO2[150].tg.push_back("PLUS");
	GOTO2[150].tg.push_back("TIMES");
	GOTO2[150].tg.push_back("SEMICOLON");
	GOTO2[150].next.push_back(96);
	GOTO2[150].next.push_back(97);
	GOTO2[150].next.push_back(98);
	GOTO2[150].next.push_back(99);
	GOTO2[150].next.push_back(100);
	GOTO2[150].next.push_back(101);
	GOTO2[150].next.push_back(102);
	GOTO2[150].next.push_back(-41);
	GOTO2[151].tg.push_back("DIVIDE");
	GOTO2[151].tg.push_back("DOT");
	GOTO2[151].tg.push_back("EQUAL");
	GOTO2[151].tg.push_back("LBRACE");
	GOTO2[151].tg.push_back("MINUS");
	GOTO2[151].tg.push_back("PLUS");
	GOTO2[151].tg.push_back("TIMES");
	GOTO2[151].tg.push_back("SEMICOLON");
	GOTO2[151].next.push_back(96);
	GOTO2[151].next.push_back(97);
	GOTO2[151].next.push_back(98);
	GOTO2[151].next.push_back(99);
	GOTO2[151].next.push_back(100);
	GOTO2[151].next.push_back(101);
	GOTO2[151].next.push_back(102);
	GOTO2[151].next.push_back(-43);
	GOTO2[152].tg.push_back("DIVIDE");
	GOTO2[152].tg.push_back("DOT");
	GOTO2[152].tg.push_back("EQUAL");
	GOTO2[152].tg.push_back("LBRACE");
	GOTO2[152].tg.push_back("MINUS");
	GOTO2[152].tg.push_back("PLUS");
	GOTO2[152].tg.push_back("TIMES");
	GOTO2[152].tg.push_back("RPAR");
	GOTO2[152].next.push_back(127);
	GOTO2[152].next.push_back(128);
	GOTO2[152].next.push_back(129);
	GOTO2[152].next.push_back(130);
	GOTO2[152].next.push_back(131);
	GOTO2[152].next.push_back(132);
	GOTO2[152].next.push_back(134);
	GOTO2[152].next.push_back(197);
	GOTO2[153].tg.push_back("DIVIDE");
	GOTO2[153].tg.push_back("DOT");
	GOTO2[153].tg.push_back("EQUAL");
	GOTO2[153].tg.push_back("LBRACE");
	GOTO2[153].tg.push_back("MINUS");
	GOTO2[153].tg.push_back("PLUS");
	GOTO2[153].tg.push_back("TIMES");
	GOTO2[153].tg.push_back("RBRACE");
	GOTO2[153].next.push_back(155);
	GOTO2[153].next.push_back(156);
	GOTO2[153].next.push_back(157);
	GOTO2[153].next.push_back(158);
	GOTO2[153].next.push_back(159);
	GOTO2[153].next.push_back(160);
	GOTO2[153].next.push_back(162);
	GOTO2[153].next.push_back(-47);
	GOTO2[154].tg.push_back("LPAR");
	GOTO2[154].tg.push_back("MINUS");
	GOTO2[154].tg.push_back("NAME");
	GOTO2[154].tg.push_back("NUMBER");
	GOTO2[154].tg.push_back("exp");
	GOTO2[154].tg.push_back("var");
	GOTO2[154].tg.push_back("RPAR");
	GOTO2[154].tg.push_back("exps");
	GOTO2[154].next.push_back(137);
	GOTO2[154].next.push_back(138);
	GOTO2[154].next.push_back(139);
	GOTO2[154].next.push_back(140);
	GOTO2[154].next.push_back(142);
	GOTO2[154].next.push_back(144);
	GOTO2[154].next.push_back(198);
	GOTO2[154].next.push_back(199);
	GOTO2[155].tg.push_back("LPAR");
	GOTO2[155].tg.push_back("MINUS");
	GOTO2[155].tg.push_back("NAME");
	GOTO2[155].tg.push_back("NUMBER");
	GOTO2[155].tg.push_back("var");
	GOTO2[155].tg.push_back("exp");
	GOTO2[155].next.push_back(105);
	GOTO2[155].next.push_back(106);
	GOTO2[155].next.push_back(107);
	GOTO2[155].next.push_back(108);
	GOTO2[155].next.push_back(110);
	GOTO2[155].next.push_back(200);
	GOTO2[156].tg.push_back("NAME");
	GOTO2[156].next.push_back(201);
	GOTO2[157].tg.push_back("LPAR");
	GOTO2[157].tg.push_back("MINUS");
	GOTO2[157].tg.push_back("NAME");
	GOTO2[157].tg.push_back("NUMBER");
	GOTO2[157].tg.push_back("var");
	GOTO2[157].tg.push_back("exp");
	GOTO2[157].next.push_back(105);
	GOTO2[157].next.push_back(106);
	GOTO2[157].next.push_back(107);
	GOTO2[157].next.push_back(108);
	GOTO2[157].next.push_back(110);
	GOTO2[157].next.push_back(202);
	GOTO2[158].tg.push_back("LPAR");
	GOTO2[158].tg.push_back("MINUS");
	GOTO2[158].tg.push_back("NAME");
	GOTO2[158].tg.push_back("NUMBER");
	GOTO2[158].tg.push_back("var");
	GOTO2[158].tg.push_back("exp");
	GOTO2[158].next.push_back(105);
	GOTO2[158].next.push_back(106);
	GOTO2[158].next.push_back(107);
	GOTO2[158].next.push_back(108);
	GOTO2[158].next.push_back(110);
	GOTO2[158].next.push_back(203);
	GOTO2[159].tg.push_back("LPAR");
	GOTO2[159].tg.push_back("MINUS");
	GOTO2[159].tg.push_back("NAME");
	GOTO2[159].tg.push_back("NUMBER");
	GOTO2[159].tg.push_back("var");
	GOTO2[159].tg.push_back("exp");
	GOTO2[159].next.push_back(105);
	GOTO2[159].next.push_back(106);
	GOTO2[159].next.push_back(107);
	GOTO2[159].next.push_back(108);
	GOTO2[159].next.push_back(110);
	GOTO2[159].next.push_back(204);
	GOTO2[160].tg.push_back("LPAR");
	GOTO2[160].tg.push_back("MINUS");
	GOTO2[160].tg.push_back("NAME");
	GOTO2[160].tg.push_back("NUMBER");
	GOTO2[160].tg.push_back("var");
	GOTO2[160].tg.push_back("exp");
	GOTO2[160].next.push_back(105);
	GOTO2[160].next.push_back(106);
	GOTO2[160].next.push_back(107);
	GOTO2[160].next.push_back(108);
	GOTO2[160].next.push_back(110);
	GOTO2[160].next.push_back(205);
	GOTO2[161].tg.push_back("ASSIGN");
	GOTO2[161].tg.push_back("DOT");
	GOTO2[161].tg.push_back("LBRACE");
	GOTO2[161].next.push_back(-34);
	GOTO2[161].next.push_back(-34);
	GOTO2[161].next.push_back(-34);
	GOTO2[162].tg.push_back("LPAR");
	GOTO2[162].tg.push_back("MINUS");
	GOTO2[162].tg.push_back("NAME");
	GOTO2[162].tg.push_back("NUMBER");
	GOTO2[162].tg.push_back("var");
	GOTO2[162].tg.push_back("exp");
	GOTO2[162].next.push_back(105);
	GOTO2[162].next.push_back(106);
	GOTO2[162].next.push_back(107);
	GOTO2[162].next.push_back(108);
	GOTO2[162].next.push_back(110);
	GOTO2[162].next.push_back(206);
	GOTO2[163].tg.push_back("ELSE");
	GOTO2[163].tg.push_back("SEMICOLON");
	GOTO2[163].next.push_back(-12);
	GOTO2[163].next.push_back(-12);
	GOTO2[164].tg.push_back("DIVIDE");
	GOTO2[164].tg.push_back("DOT");
	GOTO2[164].tg.push_back("ELSE");
	GOTO2[164].tg.push_back("EQUAL");
	GOTO2[164].tg.push_back("LBRACE");
	GOTO2[164].tg.push_back("MINUS");
	GOTO2[164].tg.push_back("PLUS");
	GOTO2[164].tg.push_back("SEMICOLON");
	GOTO2[164].tg.push_back("TIMES");
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[164].next.push_back(-46);
	GOTO2[165].tg.push_back("DIVIDE");
	GOTO2[165].tg.push_back("DOT");
	GOTO2[165].tg.push_back("ELSE");
	GOTO2[165].tg.push_back("EQUAL");
	GOTO2[165].tg.push_back("LBRACE");
	GOTO2[165].tg.push_back("MINUS");
	GOTO2[165].tg.push_back("PLUS");
	GOTO2[165].tg.push_back("SEMICOLON");
	GOTO2[165].tg.push_back("TIMES");
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[165].next.push_back(-50);
	GOTO2[166].tg.push_back("RPAR");
	GOTO2[166].next.push_back(207);
	GOTO2[167].tg.push_back("DIVIDE");
	GOTO2[167].tg.push_back("DOT");
	GOTO2[167].tg.push_back("EQUAL");
	GOTO2[167].tg.push_back("LBRACE");
	GOTO2[167].tg.push_back("MINUS");
	GOTO2[167].tg.push_back("PLUS");
	GOTO2[167].tg.push_back("TIMES");
	GOTO2[167].tg.push_back("ELSE");
	GOTO2[167].tg.push_back("SEMICOLON");
	GOTO2[167].next.push_back(116);
	GOTO2[167].next.push_back(117);
	GOTO2[167].next.push_back(118);
	GOTO2[167].next.push_back(119);
	GOTO2[167].next.push_back(120);
	GOTO2[167].next.push_back(121);
	GOTO2[167].next.push_back(122);
	GOTO2[167].next.push_back(-44);
	GOTO2[167].next.push_back(-44);
	GOTO2[168].tg.push_back("DIVIDE");
	GOTO2[168].tg.push_back("DOT");
	GOTO2[168].tg.push_back("ELSE");
	GOTO2[168].tg.push_back("EQUAL");
	GOTO2[168].tg.push_back("LBRACE");
	GOTO2[168].tg.push_back("MINUS");
	GOTO2[168].tg.push_back("PLUS");
	GOTO2[168].tg.push_back("SEMICOLON");
	GOTO2[168].tg.push_back("TIMES");
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[168].next.push_back(-39);
	GOTO2[169].tg.push_back("DIVIDE");
	GOTO2[169].tg.push_back("DOT");
	GOTO2[169].tg.push_back("EQUAL");
	GOTO2[169].tg.push_back("LBRACE");
	GOTO2[169].tg.push_back("MINUS");
	GOTO2[169].tg.push_back("PLUS");
	GOTO2[169].tg.push_back("TIMES");
	GOTO2[169].tg.push_back("ELSE");
	GOTO2[169].tg.push_back("SEMICOLON");
	GOTO2[169].next.push_back(116);
	GOTO2[169].next.push_back(117);
	GOTO2[169].next.push_back(118);
	GOTO2[169].next.push_back(119);
	GOTO2[169].next.push_back(120);
	GOTO2[169].next.push_back(121);
	GOTO2[169].next.push_back(122);
	GOTO2[169].next.push_back(-45);
	GOTO2[169].next.push_back(-45);
	GOTO2[170].tg.push_back("DIVIDE");
	GOTO2[170].tg.push_back("DOT");
	GOTO2[170].tg.push_back("EQUAL");
	GOTO2[170].tg.push_back("LBRACE");
	GOTO2[170].tg.push_back("MINUS");
	GOTO2[170].tg.push_back("PLUS");
	GOTO2[170].tg.push_back("TIMES");
	GOTO2[170].tg.push_back("RBRACE");
	GOTO2[170].next.push_back(155);
	GOTO2[170].next.push_back(156);
	GOTO2[170].next.push_back(157);
	GOTO2[170].next.push_back(158);
	GOTO2[170].next.push_back(159);
	GOTO2[170].next.push_back(160);
	GOTO2[170].next.push_back(162);
	GOTO2[170].next.push_back(208);
	GOTO2[171].tg.push_back("DIVIDE");
	GOTO2[171].tg.push_back("DOT");
	GOTO2[171].tg.push_back("EQUAL");
	GOTO2[171].tg.push_back("LBRACE");
	GOTO2[171].tg.push_back("MINUS");
	GOTO2[171].tg.push_back("PLUS");
	GOTO2[171].tg.push_back("TIMES");
	GOTO2[171].tg.push_back("ELSE");
	GOTO2[171].tg.push_back("SEMICOLON");
	GOTO2[171].next.push_back(116);
	GOTO2[171].next.push_back(117);
	GOTO2[171].next.push_back(118);
	GOTO2[171].next.push_back(119);
	GOTO2[171].next.push_back(120);
	GOTO2[171].next.push_back(121);
	GOTO2[171].next.push_back(122);
	GOTO2[171].next.push_back(-42);
	GOTO2[171].next.push_back(-42);
	GOTO2[172].tg.push_back("DIVIDE");
	GOTO2[172].tg.push_back("DOT");
	GOTO2[172].tg.push_back("EQUAL");
	GOTO2[172].tg.push_back("LBRACE");
	GOTO2[172].tg.push_back("MINUS");
	GOTO2[172].tg.push_back("PLUS");
	GOTO2[172].tg.push_back("TIMES");
	GOTO2[172].tg.push_back("ELSE");
	GOTO2[172].tg.push_back("SEMICOLON");
	GOTO2[172].next.push_back(116);
	GOTO2[172].next.push_back(117);
	GOTO2[172].next.push_back(118);
	GOTO2[172].next.push_back(119);
	GOTO2[172].next.push_back(120);
	GOTO2[172].next.push_back(121);
	GOTO2[172].next.push_back(122);
	GOTO2[172].next.push_back(-41);
	GOTO2[172].next.push_back(-41);
	GOTO2[173].tg.push_back("DIVIDE");
	GOTO2[173].tg.push_back("DOT");
	GOTO2[173].tg.push_back("EQUAL");
	GOTO2[173].tg.push_back("LBRACE");
	GOTO2[173].tg.push_back("MINUS");
	GOTO2[173].tg.push_back("PLUS");
	GOTO2[173].tg.push_back("TIMES");
	GOTO2[173].tg.push_back("ELSE");
	GOTO2[173].tg.push_back("SEMICOLON");
	GOTO2[173].next.push_back(116);
	GOTO2[173].next.push_back(117);
	GOTO2[173].next.push_back(118);
	GOTO2[173].next.push_back(119);
	GOTO2[173].next.push_back(120);
	GOTO2[173].next.push_back(121);
	GOTO2[173].next.push_back(122);
	GOTO2[173].next.push_back(-43);
	GOTO2[173].next.push_back(-43);
	GOTO2[174].tg.push_back("DIVIDE");
	GOTO2[174].tg.push_back("DOT");
	GOTO2[174].tg.push_back("EQUAL");
	GOTO2[174].tg.push_back("LBRACE");
	GOTO2[174].tg.push_back("MINUS");
	GOTO2[174].tg.push_back("PLUS");
	GOTO2[174].tg.push_back("RPAR");
	GOTO2[174].tg.push_back("TIMES");
	GOTO2[174].next.push_back(-46);
	GOTO2[174].next.push_back(-46);
	GOTO2[174].next.push_back(-46);
	GOTO2[174].next.push_back(-46);
	GOTO2[174].next.push_back(-46);
	GOTO2[174].next.push_back(-46);
	GOTO2[174].next.push_back(-46);
	GOTO2[174].next.push_back(-46);
	GOTO2[175].tg.push_back("DIVIDE");
	GOTO2[175].tg.push_back("DOT");
	GOTO2[175].tg.push_back("EQUAL");
	GOTO2[175].tg.push_back("LBRACE");
	GOTO2[175].tg.push_back("MINUS");
	GOTO2[175].tg.push_back("PLUS");
	GOTO2[175].tg.push_back("RPAR");
	GOTO2[175].tg.push_back("TIMES");
	GOTO2[175].next.push_back(-50);
	GOTO2[175].next.push_back(-50);
	GOTO2[175].next.push_back(-50);
	GOTO2[175].next.push_back(-50);
	GOTO2[175].next.push_back(-50);
	GOTO2[175].next.push_back(-50);
	GOTO2[175].next.push_back(-50);
	GOTO2[175].next.push_back(-50);
	GOTO2[176].tg.push_back("RPAR");
	GOTO2[176].next.push_back(209);
	GOTO2[177].tg.push_back("DIVIDE");
	GOTO2[177].tg.push_back("DOT");
	GOTO2[177].tg.push_back("EQUAL");
	GOTO2[177].tg.push_back("LBRACE");
	GOTO2[177].tg.push_back("MINUS");
	GOTO2[177].tg.push_back("PLUS");
	GOTO2[177].tg.push_back("TIMES");
	GOTO2[177].tg.push_back("RPAR");
	GOTO2[177].next.push_back(127);
	GOTO2[177].next.push_back(128);
	GOTO2[177].next.push_back(129);
	GOTO2[177].next.push_back(130);
	GOTO2[177].next.push_back(131);
	GOTO2[177].next.push_back(132);
	GOTO2[177].next.push_back(134);
	GOTO2[177].next.push_back(-44);
	GOTO2[178].tg.push_back("DIVIDE");
	GOTO2[178].tg.push_back("DOT");
	GOTO2[178].tg.push_back("EQUAL");
	GOTO2[178].tg.push_back("LBRACE");
	GOTO2[178].tg.push_back("MINUS");
	GOTO2[178].tg.push_back("PLUS");
	GOTO2[178].tg.push_back("RPAR");
	GOTO2[178].tg.push_back("TIMES");
	GOTO2[178].next.push_back(-39);
	GOTO2[178].next.push_back(-39);
	GOTO2[178].next.push_back(-39);
	GOTO2[178].next.push_back(-39);
	GOTO2[178].next.push_back(-39);
	GOTO2[178].next.push_back(-39);
	GOTO2[178].next.push_back(-39);
	GOTO2[178].next.push_back(-39);
	GOTO2[179].tg.push_back("DIVIDE");
	GOTO2[179].tg.push_back("DOT");
	GOTO2[179].tg.push_back("EQUAL");
	GOTO2[179].tg.push_back("LBRACE");
	GOTO2[179].tg.push_back("MINUS");
	GOTO2[179].tg.push_back("PLUS");
	GOTO2[179].tg.push_back("TIMES");
	GOTO2[179].tg.push_back("RPAR");
	GOTO2[179].next.push_back(127);
	GOTO2[179].next.push_back(128);
	GOTO2[179].next.push_back(129);
	GOTO2[179].next.push_back(130);
	GOTO2[179].next.push_back(131);
	GOTO2[179].next.push_back(132);
	GOTO2[179].next.push_back(134);
	GOTO2[179].next.push_back(-45);
	GOTO2[180].tg.push_back("DIVIDE");
	GOTO2[180].tg.push_back("DOT");
	GOTO2[180].tg.push_back("EQUAL");
	GOTO2[180].tg.push_back("LBRACE");
	GOTO2[180].tg.push_back("MINUS");
	GOTO2[180].tg.push_back("PLUS");
	GOTO2[180].tg.push_back("TIMES");
	GOTO2[180].tg.push_back("RBRACE");
	GOTO2[180].next.push_back(155);
	GOTO2[180].next.push_back(156);
	GOTO2[180].next.push_back(157);
	GOTO2[180].next.push_back(158);
	GOTO2[180].next.push_back(159);
	GOTO2[180].next.push_back(160);
	GOTO2[180].next.push_back(162);
	GOTO2[180].next.push_back(210);
	GOTO2[181].tg.push_back("DIVIDE");
	GOTO2[181].tg.push_back("DOT");
	GOTO2[181].tg.push_back("EQUAL");
	GOTO2[181].tg.push_back("LBRACE");
	GOTO2[181].tg.push_back("MINUS");
	GOTO2[181].tg.push_back("PLUS");
	GOTO2[181].tg.push_back("TIMES");
	GOTO2[181].tg.push_back("RPAR");
	GOTO2[181].next.push_back(127);
	GOTO2[181].next.push_back(128);
	GOTO2[181].next.push_back(129);
	GOTO2[181].next.push_back(130);
	GOTO2[181].next.push_back(131);
	GOTO2[181].next.push_back(132);
	GOTO2[181].next.push_back(134);
	GOTO2[181].next.push_back(-42);
	GOTO2[182].tg.push_back("DIVIDE");
	GOTO2[182].tg.push_back("DOT");
	GOTO2[182].tg.push_back("EQUAL");
	GOTO2[182].tg.push_back("LBRACE");
	GOTO2[182].tg.push_back("MINUS");
	GOTO2[182].tg.push_back("PLUS");
	GOTO2[182].tg.push_back("TIMES");
	GOTO2[182].tg.push_back("RPAR");
	GOTO2[182].next.push_back(127);
	GOTO2[182].next.push_back(128);
	GOTO2[182].next.push_back(129);
	GOTO2[182].next.push_back(130);
	GOTO2[182].next.push_back(131);
	GOTO2[182].next.push_back(132);
	GOTO2[182].next.push_back(134);
	GOTO2[182].next.push_back(-41);
	GOTO2[183].tg.push_back("DIVIDE");
	GOTO2[183].tg.push_back("DOT");
	GOTO2[183].tg.push_back("EQUAL");
	GOTO2[183].tg.push_back("LBRACE");
	GOTO2[183].tg.push_back("MINUS");
	GOTO2[183].tg.push_back("PLUS");
	GOTO2[183].tg.push_back("TIMES");
	GOTO2[183].tg.push_back("RPAR");
	GOTO2[183].next.push_back(127);
	GOTO2[183].next.push_back(128);
	GOTO2[183].next.push_back(129);
	GOTO2[183].next.push_back(130);
	GOTO2[183].next.push_back(131);
	GOTO2[183].next.push_back(132);
	GOTO2[183].next.push_back(134);
	GOTO2[183].next.push_back(-43);
	GOTO2[184].tg.push_back("DIVIDE");
	GOTO2[184].tg.push_back("DOT");
	GOTO2[184].tg.push_back("EQUAL");
	GOTO2[184].tg.push_back("LBRACE");
	GOTO2[184].tg.push_back("MINUS");
	GOTO2[184].tg.push_back("PLUS");
	GOTO2[184].tg.push_back("TIMES");
	GOTO2[184].tg.push_back("RPAR");
	GOTO2[184].next.push_back(127);
	GOTO2[184].next.push_back(128);
	GOTO2[184].next.push_back(129);
	GOTO2[184].next.push_back(130);
	GOTO2[184].next.push_back(131);
	GOTO2[184].next.push_back(132);
	GOTO2[184].next.push_back(134);
	GOTO2[184].next.push_back(211);
	GOTO2[185].tg.push_back("DIVIDE");
	GOTO2[185].tg.push_back("DOT");
	GOTO2[185].tg.push_back("EQUAL");
	GOTO2[185].tg.push_back("LBRACE");
	GOTO2[185].tg.push_back("MINUS");
	GOTO2[185].tg.push_back("PLUS");
	GOTO2[185].tg.push_back("TIMES");
	GOTO2[185].tg.push_back("COMMA");
	GOTO2[185].tg.push_back("RPAR");
	GOTO2[185].next.push_back(188);
	GOTO2[185].next.push_back(189);
	GOTO2[185].next.push_back(190);
	GOTO2[185].next.push_back(191);
	GOTO2[185].next.push_back(192);
	GOTO2[185].next.push_back(193);
	GOTO2[185].next.push_back(194);
	GOTO2[185].next.push_back(-47);
	GOTO2[185].next.push_back(-47);
	GOTO2[186].tg.push_back("LPAR");
	GOTO2[186].tg.push_back("MINUS");
	GOTO2[186].tg.push_back("NAME");
	GOTO2[186].tg.push_back("NUMBER");
	GOTO2[186].tg.push_back("exp");
	GOTO2[186].tg.push_back("var");
	GOTO2[186].tg.push_back("RPAR");
	GOTO2[186].tg.push_back("exps");
	GOTO2[186].next.push_back(137);
	GOTO2[186].next.push_back(138);
	GOTO2[186].next.push_back(139);
	GOTO2[186].next.push_back(140);
	GOTO2[186].next.push_back(142);
	GOTO2[186].next.push_back(144);
	GOTO2[186].next.push_back(212);
	GOTO2[186].next.push_back(213);
	GOTO2[187].tg.push_back("LPAR");
	GOTO2[187].tg.push_back("MINUS");
	GOTO2[187].tg.push_back("NAME");
	GOTO2[187].tg.push_back("NUMBER");
	GOTO2[187].tg.push_back("exp");
	GOTO2[187].tg.push_back("var");
	GOTO2[187].tg.push_back("exps");
	GOTO2[187].next.push_back(137);
	GOTO2[187].next.push_back(138);
	GOTO2[187].next.push_back(139);
	GOTO2[187].next.push_back(140);
	GOTO2[187].next.push_back(142);
	GOTO2[187].next.push_back(144);
	GOTO2[187].next.push_back(214);
	GOTO2[188].tg.push_back("LPAR");
	GOTO2[188].tg.push_back("MINUS");
	GOTO2[188].tg.push_back("NAME");
	GOTO2[188].tg.push_back("NUMBER");
	GOTO2[188].tg.push_back("var");
	GOTO2[188].tg.push_back("exp");
	GOTO2[188].next.push_back(137);
	GOTO2[188].next.push_back(138);
	GOTO2[188].next.push_back(139);
	GOTO2[188].next.push_back(140);
	GOTO2[188].next.push_back(144);
	GOTO2[188].next.push_back(215);
	GOTO2[189].tg.push_back("NAME");
	GOTO2[189].next.push_back(216);
	GOTO2[190].tg.push_back("LPAR");
	GOTO2[190].tg.push_back("MINUS");
	GOTO2[190].tg.push_back("NAME");
	GOTO2[190].tg.push_back("NUMBER");
	GOTO2[190].tg.push_back("var");
	GOTO2[190].tg.push_back("exp");
	GOTO2[190].next.push_back(137);
	GOTO2[190].next.push_back(138);
	GOTO2[190].next.push_back(139);
	GOTO2[190].next.push_back(140);
	GOTO2[190].next.push_back(144);
	GOTO2[190].next.push_back(217);
	GOTO2[191].tg.push_back("LPAR");
	GOTO2[191].tg.push_back("MINUS");
	GOTO2[191].tg.push_back("NAME");
	GOTO2[191].tg.push_back("NUMBER");
	GOTO2[191].tg.push_back("var");
	GOTO2[191].tg.push_back("exp");
	GOTO2[191].next.push_back(105);
	GOTO2[191].next.push_back(106);
	GOTO2[191].next.push_back(107);
	GOTO2[191].next.push_back(108);
	GOTO2[191].next.push_back(110);
	GOTO2[191].next.push_back(218);
	GOTO2[192].tg.push_back("LPAR");
	GOTO2[192].tg.push_back("MINUS");
	GOTO2[192].tg.push_back("NAME");
	GOTO2[192].tg.push_back("NUMBER");
	GOTO2[192].tg.push_back("var");
	GOTO2[192].tg.push_back("exp");
	GOTO2[192].next.push_back(137);
	GOTO2[192].next.push_back(138);
	GOTO2[192].next.push_back(139);
	GOTO2[192].next.push_back(140);
	GOTO2[192].next.push_back(144);
	GOTO2[192].next.push_back(219);
	GOTO2[193].tg.push_back("LPAR");
	GOTO2[193].tg.push_back("MINUS");
	GOTO2[193].tg.push_back("NAME");
	GOTO2[193].tg.push_back("NUMBER");
	GOTO2[193].tg.push_back("var");
	GOTO2[193].tg.push_back("exp");
	GOTO2[193].next.push_back(137);
	GOTO2[193].next.push_back(138);
	GOTO2[193].next.push_back(139);
	GOTO2[193].next.push_back(140);
	GOTO2[193].next.push_back(144);
	GOTO2[193].next.push_back(220);
	GOTO2[194].tg.push_back("LPAR");
	GOTO2[194].tg.push_back("MINUS");
	GOTO2[194].tg.push_back("NAME");
	GOTO2[194].tg.push_back("NUMBER");
	GOTO2[194].tg.push_back("var");
	GOTO2[194].tg.push_back("exp");
	GOTO2[194].next.push_back(137);
	GOTO2[194].next.push_back(138);
	GOTO2[194].next.push_back(139);
	GOTO2[194].next.push_back(140);
	GOTO2[194].next.push_back(144);
	GOTO2[194].next.push_back(221);
	GOTO2[195].tg.push_back("DIVIDE");
	GOTO2[195].tg.push_back("DOT");
	GOTO2[195].tg.push_back("EQUAL");
	GOTO2[195].tg.push_back("LBRACE");
	GOTO2[195].tg.push_back("MINUS");
	GOTO2[195].tg.push_back("PLUS");
	GOTO2[195].tg.push_back("SEMICOLON");
	GOTO2[195].tg.push_back("TIMES");
	GOTO2[195].next.push_back(-51);
	GOTO2[195].next.push_back(-51);
	GOTO2[195].next.push_back(-51);
	GOTO2[195].next.push_back(-51);
	GOTO2[195].next.push_back(-51);
	GOTO2[195].next.push_back(-51);
	GOTO2[195].next.push_back(-51);
	GOTO2[195].next.push_back(-51);
	GOTO2[196].tg.push_back("DIVIDE");
	GOTO2[196].tg.push_back("DOT");
	GOTO2[196].tg.push_back("EQUAL");
	GOTO2[196].tg.push_back("LBRACE");
	GOTO2[196].tg.push_back("MINUS");
	GOTO2[196].tg.push_back("PLUS");
	GOTO2[196].tg.push_back("SEMICOLON");
	GOTO2[196].tg.push_back("TIMES");
	GOTO2[196].next.push_back(-40);
	GOTO2[196].next.push_back(-40);
	GOTO2[196].next.push_back(-40);
	GOTO2[196].next.push_back(-40);
	GOTO2[196].next.push_back(-40);
	GOTO2[196].next.push_back(-40);
	GOTO2[196].next.push_back(-40);
	GOTO2[196].next.push_back(-40);
	GOTO2[197].tg.push_back("DIVIDE");
	GOTO2[197].tg.push_back("DOT");
	GOTO2[197].tg.push_back("EQUAL");
	GOTO2[197].tg.push_back("LBRACE");
	GOTO2[197].tg.push_back("MINUS");
	GOTO2[197].tg.push_back("PLUS");
	GOTO2[197].tg.push_back("RBRACE");
	GOTO2[197].tg.push_back("TIMES");
	GOTO2[197].next.push_back(-46);
	GOTO2[197].next.push_back(-46);
	GOTO2[197].next.push_back(-46);
	GOTO2[197].next.push_back(-46);
	GOTO2[197].next.push_back(-46);
	GOTO2[197].next.push_back(-46);
	GOTO2[197].next.push_back(-46);
	GOTO2[197].next.push_back(-46);
	GOTO2[198].tg.push_back("DIVIDE");
	GOTO2[198].tg.push_back("DOT");
	GOTO2[198].tg.push_back("EQUAL");
	GOTO2[198].tg.push_back("LBRACE");
	GOTO2[198].tg.push_back("MINUS");
	GOTO2[198].tg.push_back("PLUS");
	GOTO2[198].tg.push_back("RBRACE");
	GOTO2[198].tg.push_back("TIMES");
	GOTO2[198].next.push_back(-50);
	GOTO2[198].next.push_back(-50);
	GOTO2[198].next.push_back(-50);
	GOTO2[198].next.push_back(-50);
	GOTO2[198].next.push_back(-50);
	GOTO2[198].next.push_back(-50);
	GOTO2[198].next.push_back(-50);
	GOTO2[198].next.push_back(-50);
	GOTO2[199].tg.push_back("RPAR");
	GOTO2[199].next.push_back(222);
	GOTO2[200].tg.push_back("DIVIDE");
	GOTO2[200].tg.push_back("DOT");
	GOTO2[200].tg.push_back("EQUAL");
	GOTO2[200].tg.push_back("LBRACE");
	GOTO2[200].tg.push_back("MINUS");
	GOTO2[200].tg.push_back("PLUS");
	GOTO2[200].tg.push_back("TIMES");
	GOTO2[200].tg.push_back("RBRACE");
	GOTO2[200].next.push_back(155);
	GOTO2[200].next.push_back(156);
	GOTO2[200].next.push_back(157);
	GOTO2[200].next.push_back(158);
	GOTO2[200].next.push_back(159);
	GOTO2[200].next.push_back(160);
	GOTO2[200].next.push_back(162);
	GOTO2[200].next.push_back(-44);
	GOTO2[201].tg.push_back("DIVIDE");
	GOTO2[201].tg.push_back("DOT");
	GOTO2[201].tg.push_back("EQUAL");
	GOTO2[201].tg.push_back("LBRACE");
	GOTO2[201].tg.push_back("MINUS");
	GOTO2[201].tg.push_back("PLUS");
	GOTO2[201].tg.push_back("RBRACE");
	GOTO2[201].tg.push_back("TIMES");
	GOTO2[201].next.push_back(-39);
	GOTO2[201].next.push_back(-39);
	GOTO2[201].next.push_back(-39);
	GOTO2[201].next.push_back(-39);
	GOTO2[201].next.push_back(-39);
	GOTO2[201].next.push_back(-39);
	GOTO2[201].next.push_back(-39);
	GOTO2[201].next.push_back(-39);
	GOTO2[202].tg.push_back("DIVIDE");
	GOTO2[202].tg.push_back("DOT");
	GOTO2[202].tg.push_back("EQUAL");
	GOTO2[202].tg.push_back("LBRACE");
	GOTO2[202].tg.push_back("MINUS");
	GOTO2[202].tg.push_back("PLUS");
	GOTO2[202].tg.push_back("TIMES");
	GOTO2[202].tg.push_back("RBRACE");
	GOTO2[202].next.push_back(155);
	GOTO2[202].next.push_back(156);
	GOTO2[202].next.push_back(157);
	GOTO2[202].next.push_back(158);
	GOTO2[202].next.push_back(159);
	GOTO2[202].next.push_back(160);
	GOTO2[202].next.push_back(162);
	GOTO2[202].next.push_back(-45);
	GOTO2[203].tg.push_back("DIVIDE");
	GOTO2[203].tg.push_back("DOT");
	GOTO2[203].tg.push_back("EQUAL");
	GOTO2[203].tg.push_back("LBRACE");
	GOTO2[203].tg.push_back("MINUS");
	GOTO2[203].tg.push_back("PLUS");
	GOTO2[203].tg.push_back("TIMES");
	GOTO2[203].tg.push_back("RBRACE");
	GOTO2[203].next.push_back(155);
	GOTO2[203].next.push_back(156);
	GOTO2[203].next.push_back(157);
	GOTO2[203].next.push_back(158);
	GOTO2[203].next.push_back(159);
	GOTO2[203].next.push_back(160);
	GOTO2[203].next.push_back(162);
	GOTO2[203].next.push_back(223);
	GOTO2[204].tg.push_back("DIVIDE");
	GOTO2[204].tg.push_back("DOT");
	GOTO2[204].tg.push_back("EQUAL");
	GOTO2[204].tg.push_back("LBRACE");
	GOTO2[204].tg.push_back("MINUS");
	GOTO2[204].tg.push_back("PLUS");
	GOTO2[204].tg.push_back("TIMES");
	GOTO2[204].tg.push_back("RBRACE");
	GOTO2[204].next.push_back(155);
	GOTO2[204].next.push_back(156);
	GOTO2[204].next.push_back(157);
	GOTO2[204].next.push_back(158);
	GOTO2[204].next.push_back(159);
	GOTO2[204].next.push_back(160);
	GOTO2[204].next.push_back(162);
	GOTO2[204].next.push_back(-42);
	GOTO2[205].tg.push_back("DIVIDE");
	GOTO2[205].tg.push_back("DOT");
	GOTO2[205].tg.push_back("EQUAL");
	GOTO2[205].tg.push_back("LBRACE");
	GOTO2[205].tg.push_back("MINUS");
	GOTO2[205].tg.push_back("PLUS");
	GOTO2[205].tg.push_back("TIMES");
	GOTO2[205].tg.push_back("RBRACE");
	GOTO2[205].next.push_back(155);
	GOTO2[205].next.push_back(156);
	GOTO2[205].next.push_back(157);
	GOTO2[205].next.push_back(158);
	GOTO2[205].next.push_back(159);
	GOTO2[205].next.push_back(160);
	GOTO2[205].next.push_back(162);
	GOTO2[205].next.push_back(-41);
	GOTO2[206].tg.push_back("DIVIDE");
	GOTO2[206].tg.push_back("DOT");
	GOTO2[206].tg.push_back("EQUAL");
	GOTO2[206].tg.push_back("LBRACE");
	GOTO2[206].tg.push_back("MINUS");
	GOTO2[206].tg.push_back("PLUS");
	GOTO2[206].tg.push_back("TIMES");
	GOTO2[206].tg.push_back("RBRACE");
	GOTO2[206].next.push_back(155);
	GOTO2[206].next.push_back(156);
	GOTO2[206].next.push_back(157);
	GOTO2[206].next.push_back(158);
	GOTO2[206].next.push_back(159);
	GOTO2[206].next.push_back(160);
	GOTO2[206].next.push_back(162);
	GOTO2[206].next.push_back(-43);
	GOTO2[207].tg.push_back("DIVIDE");
	GOTO2[207].tg.push_back("DOT");
	GOTO2[207].tg.push_back("ELSE");
	GOTO2[207].tg.push_back("EQUAL");
	GOTO2[207].tg.push_back("LBRACE");
	GOTO2[207].tg.push_back("MINUS");
	GOTO2[207].tg.push_back("PLUS");
	GOTO2[207].tg.push_back("SEMICOLON");
	GOTO2[207].tg.push_back("TIMES");
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[207].next.push_back(-51);
	GOTO2[208].tg.push_back("DIVIDE");
	GOTO2[208].tg.push_back("DOT");
	GOTO2[208].tg.push_back("ELSE");
	GOTO2[208].tg.push_back("EQUAL");
	GOTO2[208].tg.push_back("LBRACE");
	GOTO2[208].tg.push_back("MINUS");
	GOTO2[208].tg.push_back("PLUS");
	GOTO2[208].tg.push_back("SEMICOLON");
	GOTO2[208].tg.push_back("TIMES");
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[208].next.push_back(-40);
	GOTO2[209].tg.push_back("DIVIDE");
	GOTO2[209].tg.push_back("DOT");
	GOTO2[209].tg.push_back("EQUAL");
	GOTO2[209].tg.push_back("LBRACE");
	GOTO2[209].tg.push_back("MINUS");
	GOTO2[209].tg.push_back("PLUS");
	GOTO2[209].tg.push_back("RPAR");
	GOTO2[209].tg.push_back("TIMES");
	GOTO2[209].next.push_back(-51);
	GOTO2[209].next.push_back(-51);
	GOTO2[209].next.push_back(-51);
	GOTO2[209].next.push_back(-51);
	GOTO2[209].next.push_back(-51);
	GOTO2[209].next.push_back(-51);
	GOTO2[209].next.push_back(-51);
	GOTO2[209].next.push_back(-51);
	GOTO2[210].tg.push_back("DIVIDE");
	GOTO2[210].tg.push_back("DOT");
	GOTO2[210].tg.push_back("EQUAL");
	GOTO2[210].tg.push_back("LBRACE");
	GOTO2[210].tg.push_back("MINUS");
	GOTO2[210].tg.push_back("PLUS");
	GOTO2[210].tg.push_back("RPAR");
	GOTO2[210].tg.push_back("TIMES");
	GOTO2[210].next.push_back(-40);
	GOTO2[210].next.push_back(-40);
	GOTO2[210].next.push_back(-40);
	GOTO2[210].next.push_back(-40);
	GOTO2[210].next.push_back(-40);
	GOTO2[210].next.push_back(-40);
	GOTO2[210].next.push_back(-40);
	GOTO2[210].next.push_back(-40);
	GOTO2[211].tg.push_back("COMMA");
	GOTO2[211].tg.push_back("DIVIDE");
	GOTO2[211].tg.push_back("DOT");
	GOTO2[211].tg.push_back("EQUAL");
	GOTO2[211].tg.push_back("LBRACE");
	GOTO2[211].tg.push_back("MINUS");
	GOTO2[211].tg.push_back("PLUS");
	GOTO2[211].tg.push_back("RPAR");
	GOTO2[211].tg.push_back("TIMES");
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[211].next.push_back(-46);
	GOTO2[212].tg.push_back("COMMA");
	GOTO2[212].tg.push_back("DIVIDE");
	GOTO2[212].tg.push_back("DOT");
	GOTO2[212].tg.push_back("EQUAL");
	GOTO2[212].tg.push_back("LBRACE");
	GOTO2[212].tg.push_back("MINUS");
	GOTO2[212].tg.push_back("PLUS");
	GOTO2[212].tg.push_back("RPAR");
	GOTO2[212].tg.push_back("TIMES");
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[212].next.push_back(-50);
	GOTO2[213].tg.push_back("RPAR");
	GOTO2[213].next.push_back(224);
	GOTO2[214].tg.push_back("RPAR");
	GOTO2[214].next.push_back(-53);
	GOTO2[215].tg.push_back("DIVIDE");
	GOTO2[215].tg.push_back("DOT");
	GOTO2[215].tg.push_back("EQUAL");
	GOTO2[215].tg.push_back("LBRACE");
	GOTO2[215].tg.push_back("MINUS");
	GOTO2[215].tg.push_back("PLUS");
	GOTO2[215].tg.push_back("TIMES");
	GOTO2[215].tg.push_back("COMMA");
	GOTO2[215].tg.push_back("RPAR");
	GOTO2[215].next.push_back(188);
	GOTO2[215].next.push_back(189);
	GOTO2[215].next.push_back(190);
	GOTO2[215].next.push_back(191);
	GOTO2[215].next.push_back(192);
	GOTO2[215].next.push_back(193);
	GOTO2[215].next.push_back(194);
	GOTO2[215].next.push_back(-44);
	GOTO2[215].next.push_back(-44);
	GOTO2[216].tg.push_back("COMMA");
	GOTO2[216].tg.push_back("DIVIDE");
	GOTO2[216].tg.push_back("DOT");
	GOTO2[216].tg.push_back("EQUAL");
	GOTO2[216].tg.push_back("LBRACE");
	GOTO2[216].tg.push_back("MINUS");
	GOTO2[216].tg.push_back("PLUS");
	GOTO2[216].tg.push_back("RPAR");
	GOTO2[216].tg.push_back("TIMES");
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[216].next.push_back(-39);
	GOTO2[217].tg.push_back("DIVIDE");
	GOTO2[217].tg.push_back("DOT");
	GOTO2[217].tg.push_back("EQUAL");
	GOTO2[217].tg.push_back("LBRACE");
	GOTO2[217].tg.push_back("MINUS");
	GOTO2[217].tg.push_back("PLUS");
	GOTO2[217].tg.push_back("TIMES");
	GOTO2[217].tg.push_back("COMMA");
	GOTO2[217].tg.push_back("RPAR");
	GOTO2[217].next.push_back(188);
	GOTO2[217].next.push_back(189);
	GOTO2[217].next.push_back(190);
	GOTO2[217].next.push_back(191);
	GOTO2[217].next.push_back(192);
	GOTO2[217].next.push_back(193);
	GOTO2[217].next.push_back(194);
	GOTO2[217].next.push_back(-45);
	GOTO2[217].next.push_back(-45);
	GOTO2[218].tg.push_back("DIVIDE");
	GOTO2[218].tg.push_back("DOT");
	GOTO2[218].tg.push_back("EQUAL");
	GOTO2[218].tg.push_back("LBRACE");
	GOTO2[218].tg.push_back("MINUS");
	GOTO2[218].tg.push_back("PLUS");
	GOTO2[218].tg.push_back("TIMES");
	GOTO2[218].tg.push_back("RBRACE");
	GOTO2[218].next.push_back(155);
	GOTO2[218].next.push_back(156);
	GOTO2[218].next.push_back(157);
	GOTO2[218].next.push_back(158);
	GOTO2[218].next.push_back(159);
	GOTO2[218].next.push_back(160);
	GOTO2[218].next.push_back(162);
	GOTO2[218].next.push_back(225);
	GOTO2[219].tg.push_back("DIVIDE");
	GOTO2[219].tg.push_back("DOT");
	GOTO2[219].tg.push_back("EQUAL");
	GOTO2[219].tg.push_back("LBRACE");
	GOTO2[219].tg.push_back("MINUS");
	GOTO2[219].tg.push_back("PLUS");
	GOTO2[219].tg.push_back("TIMES");
	GOTO2[219].tg.push_back("COMMA");
	GOTO2[219].tg.push_back("RPAR");
	GOTO2[219].next.push_back(188);
	GOTO2[219].next.push_back(189);
	GOTO2[219].next.push_back(190);
	GOTO2[219].next.push_back(191);
	GOTO2[219].next.push_back(192);
	GOTO2[219].next.push_back(193);
	GOTO2[219].next.push_back(194);
	GOTO2[219].next.push_back(-42);
	GOTO2[219].next.push_back(-42);
	GOTO2[220].tg.push_back("DIVIDE");
	GOTO2[220].tg.push_back("DOT");
	GOTO2[220].tg.push_back("EQUAL");
	GOTO2[220].tg.push_back("LBRACE");
	GOTO2[220].tg.push_back("MINUS");
	GOTO2[220].tg.push_back("PLUS");
	GOTO2[220].tg.push_back("TIMES");
	GOTO2[220].tg.push_back("COMMA");
	GOTO2[220].tg.push_back("RPAR");
	GOTO2[220].next.push_back(188);
	GOTO2[220].next.push_back(189);
	GOTO2[220].next.push_back(190);
	GOTO2[220].next.push_back(191);
	GOTO2[220].next.push_back(192);
	GOTO2[220].next.push_back(193);
	GOTO2[220].next.push_back(194);
	GOTO2[220].next.push_back(-41);
	GOTO2[220].next.push_back(-41);
	GOTO2[221].tg.push_back("DIVIDE");
	GOTO2[221].tg.push_back("DOT");
	GOTO2[221].tg.push_back("EQUAL");
	GOTO2[221].tg.push_back("LBRACE");
	GOTO2[221].tg.push_back("MINUS");
	GOTO2[221].tg.push_back("PLUS");
	GOTO2[221].tg.push_back("TIMES");
	GOTO2[221].tg.push_back("COMMA");
	GOTO2[221].tg.push_back("RPAR");
	GOTO2[221].next.push_back(188);
	GOTO2[221].next.push_back(189);
	GOTO2[221].next.push_back(190);
	GOTO2[221].next.push_back(191);
	GOTO2[221].next.push_back(192);
	GOTO2[221].next.push_back(193);
	GOTO2[221].next.push_back(194);
	GOTO2[221].next.push_back(-43);
	GOTO2[221].next.push_back(-43);
	GOTO2[222].tg.push_back("DIVIDE");
	GOTO2[222].tg.push_back("DOT");
	GOTO2[222].tg.push_back("EQUAL");
	GOTO2[222].tg.push_back("LBRACE");
	GOTO2[222].tg.push_back("MINUS");
	GOTO2[222].tg.push_back("PLUS");
	GOTO2[222].tg.push_back("RBRACE");
	GOTO2[222].tg.push_back("TIMES");
	GOTO2[222].next.push_back(-51);
	GOTO2[222].next.push_back(-51);
	GOTO2[222].next.push_back(-51);
	GOTO2[222].next.push_back(-51);
	GOTO2[222].next.push_back(-51);
	GOTO2[222].next.push_back(-51);
	GOTO2[222].next.push_back(-51);
	GOTO2[222].next.push_back(-51);
	GOTO2[223].tg.push_back("DIVIDE");
	GOTO2[223].tg.push_back("DOT");
	GOTO2[223].tg.push_back("EQUAL");
	GOTO2[223].tg.push_back("LBRACE");
	GOTO2[223].tg.push_back("MINUS");
	GOTO2[223].tg.push_back("PLUS");
	GOTO2[223].tg.push_back("RBRACE");
	GOTO2[223].tg.push_back("TIMES");
	GOTO2[223].next.push_back(-40);
	GOTO2[223].next.push_back(-40);
	GOTO2[223].next.push_back(-40);
	GOTO2[223].next.push_back(-40);
	GOTO2[223].next.push_back(-40);
	GOTO2[223].next.push_back(-40);
	GOTO2[223].next.push_back(-40);
	GOTO2[223].next.push_back(-40);
	GOTO2[224].tg.push_back("COMMA");
	GOTO2[224].tg.push_back("DIVIDE");
	GOTO2[224].tg.push_back("DOT");
	GOTO2[224].tg.push_back("EQUAL");
	GOTO2[224].tg.push_back("LBRACE");
	GOTO2[224].tg.push_back("MINUS");
	GOTO2[224].tg.push_back("PLUS");
	GOTO2[224].tg.push_back("RPAR");
	GOTO2[224].tg.push_back("TIMES");
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[224].next.push_back(-51);
	GOTO2[225].tg.push_back("COMMA");
	GOTO2[225].tg.push_back("DIVIDE");
	GOTO2[225].tg.push_back("DOT");
	GOTO2[225].tg.push_back("EQUAL");
	GOTO2[225].tg.push_back("LBRACE");
	GOTO2[225].tg.push_back("MINUS");
	GOTO2[225].tg.push_back("PLUS");
	GOTO2[225].tg.push_back("RPAR");
	GOTO2[225].tg.push_back("TIMES");
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	GOTO2[225].next.push_back(-40);
	productions[1].leftp = "program";
	productions[1].numofrights = 1;
	productions[1].ifsubp = 0;
	productions[1].subp = "";
	productions[1].rightp[0] = "declarations";
	productions[2].leftp = "declarations";
	productions[2].numofrights = 2;
	productions[2].ifsubp = 0;
	productions[2].subp = "";
	productions[2].rightp[0] = "declaration";
	productions[2].rightp[1] = "declarations";
	productions[3].leftp = "declarations";
	productions[3].numofrights = 1;
	productions[3].ifsubp = 0;
	productions[3].subp = "";
	productions[3].rightp[0] = "epsilon";
	productions[4].leftp = "declaration";
	productions[4].numofrights = 1;
	productions[4].ifsubp = 0;
	productions[4].subp = "";
	productions[4].rightp[0] = "fun_declaration";
	productions[5].leftp = "declaration";
	productions[5].numofrights = 1;
	productions[5].ifsubp = 0;
	productions[5].subp = "";
	productions[5].rightp[0] = "var_declaration";
	productions[6].leftp = "fun_declaration";
	productions[6].numofrights = 6;
	productions[6].ifsubp = 0;
	productions[6].subp = "";
	productions[6].rightp[0] = "type";
	productions[6].rightp[1] = "NAME";
	productions[6].rightp[2] = "LPAR";
	productions[6].rightp[3] = "parameters";
	productions[6].rightp[4] = "RPAR";
	productions[6].rightp[5] = "block";
	productions[7].leftp = "parameters";
	productions[7].numofrights = 1;
	productions[7].ifsubp = 0;
	productions[7].subp = "";
	productions[7].rightp[0] = "more_parameters";
	productions[8].leftp = "parameters";
	productions[8].numofrights = 1;
	productions[8].ifsubp = 0;
	productions[8].subp = "";
	productions[8].rightp[0] = "epsilon";
	productions[9].leftp = "more_parameters";
	productions[9].numofrights = 3;
	productions[9].ifsubp = 0;
	productions[9].subp = "";
	productions[9].rightp[0] = "parameter";
	productions[9].rightp[1] = "COMMA";
	productions[9].rightp[2] = "more_parameters";
	productions[10].leftp = "more_parameters";
	productions[10].numofrights = 1;
	productions[10].ifsubp = 0;
	productions[10].subp = "";
	productions[10].rightp[0] = "parameter";
	productions[11].leftp = "parameter";
	productions[11].numofrights = 2;
	productions[11].ifsubp = 0;
	productions[11].subp = "";
	productions[11].rightp[0] = "type";
	productions[11].rightp[1] = "NAME";
	productions[12].leftp = "block";
	productions[12].numofrights = 4;
	productions[12].ifsubp = 0;
	productions[12].subp = "";
	productions[12].rightp[0] = "LBRACE";
	productions[12].rightp[1] = "var_declarations";
	productions[12].rightp[2] = "statements";
	productions[12].rightp[3] = "RBRACE";
	productions[13].leftp = "var_declarations";
	productions[13].numofrights = 2;
	productions[13].ifsubp = 0;
	productions[13].subp = "";
	productions[13].rightp[0] = "var_declaration";
	productions[13].rightp[1] = "var_declarations";
	productions[14].leftp = "var_declarations";
	productions[14].numofrights = 1;
	productions[14].ifsubp = 0;
	productions[14].subp = "";
	productions[14].rightp[0] = "epsilon";
	productions[15].leftp = "var_declaration";
	productions[15].numofrights = 3;
	productions[15].ifsubp = 1;
	productions[15].subp = "3,@,@,res;";
	productions[15].rightp[0] = "type";
	productions[15].rightp[1] = "NAME";
	productions[15].rightp[2] = "SEMICOLON";
	productions[16].leftp = "type";
	productions[16].numofrights = 1;
	productions[16].ifsubp = 1;
	productions[16].subp = "2,@,@,res;";
	productions[16].rightp[0] = "INT";
	productions[17].leftp = "type";
	productions[17].numofrights = 1;
	productions[17].ifsubp = 1;
	productions[17].subp = "2,@,@,res;";
	productions[17].rightp[0] = "FLOAT";
	productions[18].leftp = "type";
	productions[18].numofrights = 2;
	productions[18].ifsubp = 0;
	productions[18].subp = "";
	productions[18].rightp[0] = "type";
	productions[18].rightp[1] = "TIMES";
	productions[19].leftp = "type";
	productions[19].numofrights = 4;
	productions[19].ifsubp = 0;
	productions[19].subp = "";
	productions[19].rightp[0] = "STRUCT";
	productions[19].rightp[1] = "LBRACE";
	productions[19].rightp[2] = "fields";
	productions[19].rightp[3] = "RBRACE";
	productions[20].leftp = "fields";
	productions[20].numofrights = 2;
	productions[20].ifsubp = 0;
	productions[20].subp = "";
	productions[20].rightp[0] = "field";
	productions[20].rightp[1] = "fields";
	productions[21].leftp = "fields";
	productions[21].numofrights = 1;
	productions[21].ifsubp = 0;
	productions[21].subp = "";
	productions[21].rightp[0] = "epsilon";
	productions[22].leftp = "field";
	productions[22].numofrights = 3;
	productions[22].ifsubp = 0;
	productions[22].subp = "";
	productions[22].rightp[0] = "type";
	productions[22].rightp[1] = "NAME";
	productions[22].rightp[2] = "SEMICOLON";
	productions[23].leftp = "statements";
	productions[23].numofrights = 3;
	productions[23].ifsubp = 0;
	productions[23].subp = "";
	productions[23].rightp[0] = "statement";
	productions[23].rightp[1] = "SEMICOLON";
	productions[23].rightp[2] = "statements";
	productions[24].leftp = "statements";
	productions[24].numofrights = 1;
	productions[24].ifsubp = 0;
	productions[24].subp = "";
	productions[24].rightp[0] = "epsilon";
	//2021 1.8
	productions[25].leftp = "statement";
	productions[25].numofrights = 2;
	productions[25].ifsubp = 1;
	productions[25].subp = "7,@,@,res;";
	productions[25].rightp[0] = "T";
	productions[25].rightp[1] = "statement";
	productions[26].leftp = "statement";
	productions[26].numofrights = 2;
	productions[26].ifsubp = 0;
	productions[26].subp = "";
	productions[26].rightp[0] = "C";
	productions[26].rightp[1] = "statement";
	productions[27].leftp = "statement";
	productions[27].numofrights = 3;
	productions[27].ifsubp = 1;
	productions[27].subp = "=,exp.val(),@,lexp.val();";
	productions[27].rightp[0] = "lexp";
	productions[27].rightp[1] = "ASSIGN";
	productions[27].rightp[2] = "exp";
	productions[28].leftp = "statement";
	productions[28].numofrights = 2;
	productions[28].ifsubp = 0;
	productions[28].subp = "";
	productions[28].rightp[0] = "RETURN";
	productions[28].rightp[1] = "exp";
	productions[29].leftp = "statement";
	productions[29].numofrights = 1;
	productions[29].ifsubp = 0;
	productions[29].subp = "";
	productions[29].rightp[0] = "block";
	productions[30].leftp = "statement";
	productions[30].numofrights = 2;
	productions[30].ifsubp = 0;
	productions[30].subp = "";
	productions[30].rightp[0] = "T";
	productions[30].rightp[1] = "statement";
	productions[31].leftp = "T";
	productions[31].numofrights = 3;
	//2021 1.8
	productions[31].ifsubp = 1;
	productions[31].subp = "6,@,@,res;";
	productions[31].rightp[0] = "C";
	productions[31].rightp[1] = "statement";
	productions[31].rightp[2] = "ELSE";
	//2021 1.8
	productions[32].leftp = "C";
	productions[32].numofrights = 4;
	productions[32].ifsubp = 1;
	productions[32].subp = "5,@,@,res;";
	productions[32].rightp[0] = "IF";
	productions[32].rightp[1] = "LPAR";
	productions[32].rightp[2] = "exp";
	productions[32].rightp[3] = "RPAR";
	productions[33].leftp = "lexp";
	productions[33].numofrights = 1;
	productions[33].ifsubp = 0;
	productions[33].subp = "";
	productions[33].rightp[0] = "var";
	productions[34].leftp = "lexp";
	productions[34].numofrights = 4;
	productions[34].ifsubp = 0;
	productions[34].subp = "";
	productions[34].rightp[0] = "lexp";
	productions[34].rightp[1] = "LBRACE";
	productions[34].rightp[2] = "exp";
	productions[34].rightp[3] = "RBRACE";
	productions[35].leftp = "lexp";
	productions[35].numofrights = 3;
	productions[35].ifsubp = 0;
	productions[35].subp = "";
	productions[35].rightp[0] = "lexp";
	productions[35].rightp[1] = "DOT";
	productions[35].rightp[2] = "NAME";
	productions[36].leftp = "lexp";
	productions[36].numofrights = 1;
	productions[36].ifsubp = 0;
	productions[36].subp = "";
	productions[36].rightp[0] = "var";
	productions[37].leftp = "lexp";
	productions[37].numofrights = 4;
	productions[37].ifsubp = 0;
	productions[37].subp = "";
	productions[37].rightp[0] = "lexp";
	productions[37].rightp[1] = "LBRACE";
	productions[37].rightp[2] = "exp";
	productions[37].rightp[3] = "RBRACE";
	productions[38].leftp = "lexp";
	productions[38].numofrights = 3;
	productions[38].ifsubp = 0;
	productions[38].subp = "";
	productions[38].rightp[0] = "lexp";
	productions[38].rightp[1] = "DOT";
	productions[38].rightp[2] = "NAME";
	productions[39].leftp = "exp";
	productions[39].numofrights = 3;
	productions[39].ifsubp = 0;
	productions[39].subp = "";
	productions[39].rightp[0] = "exp";
	productions[39].rightp[1] = "DOT";
	productions[39].rightp[2] = "NAME";
	productions[40].leftp = "exp";
	productions[40].numofrights = 4;
	productions[40].ifsubp = 0;
	productions[40].subp = "";
	productions[40].rightp[0] = "exp";
	productions[40].rightp[1] = "LBRACE";
	productions[40].rightp[2] = "exp";
	productions[40].rightp[3] = "RBRACE";
	productions[41].leftp = "exp";
	productions[41].numofrights = 3;
	productions[41].ifsubp = 1;
	productions[41].subp = "res=newTemp();+,exp1.val(),exp2.val(),res;";
	productions[41].rightp[0] = "exp";
	productions[41].rightp[1] = "PLUS";
	productions[41].rightp[2] = "exp";
	productions[42].leftp = "exp";
	productions[42].numofrights = 3;
	productions[42].ifsubp = 1;
	productions[42].subp = "res=newTemp();-,exp1.val(),exp2.val(),res;";
	productions[42].rightp[0] = "exp";
	productions[42].rightp[1] = "MINUS";
	productions[42].rightp[2] = "exp";
	productions[43].leftp = "exp";
	productions[43].numofrights = 3;
	productions[43].ifsubp = 1;
	productions[43].subp = "res=newTemp();*,exp1.val(),exp2.val(),res;";
	productions[43].rightp[0] = "exp";
	productions[43].rightp[1] = "TIMES";
	productions[43].rightp[2] = "exp";
	productions[44].leftp = "exp";
	productions[44].numofrights = 3;
	productions[44].ifsubp = 1;
	productions[44].subp = "res=newTemp();/,exp1.val(),exp2.val(),res;";
	productions[44].rightp[0] = "exp";
	productions[44].rightp[1] = "DIVIDE";
	productions[44].rightp[2] = "exp";
	productions[45].leftp = "exp";
	productions[45].numofrights = 3;
	productions[45].ifsubp = 1;
	productions[45].subp = "4,@,@,res;";
	productions[45].rightp[0] = "exp";
	productions[45].rightp[1] = "EQUAL";
	productions[45].rightp[2] = "exp";
	productions[46].leftp = "exp";
	productions[46].numofrights = 3;
	productions[46].ifsubp = 0;
	productions[46].subp = "";
	productions[46].rightp[0] = "LPAR";
	productions[46].rightp[1] = "exp";
	productions[46].rightp[2] = "RPAR";
	productions[47].leftp = "exp";
	productions[47].numofrights = 2;
	productions[47].ifsubp = 1;
	productions[47].subp = "res=newTemp();-,exp1.val(),@,res;";
	productions[47].rightp[0] = "MINUS";
	productions[47].rightp[1] = "exp";
	productions[48].leftp = "exp";
	productions[48].numofrights = 1;
	productions[48].ifsubp = 0;
	productions[48].subp = "";
	productions[48].rightp[0] = "var";
	productions[49].leftp = "exp";
	productions[49].numofrights = 1;
	productions[49].ifsubp = 1;
	productions[49].subp = "1,@,@,res;";
	productions[49].rightp[0] = "NUMBER";
	productions[50].leftp = "exp";
	productions[50].numofrights = 3;
	productions[50].ifsubp = 0;
	productions[50].subp = "";
	productions[50].rightp[0] = "NAME";
	productions[50].rightp[1] = "LPAR";
	productions[50].rightp[2] = "RPAR";
	productions[51].leftp = "exp";
	productions[51].numofrights = 4;
	productions[51].ifsubp = 0;
	productions[51].subp = "";
	productions[51].rightp[0] = "NAME";
	productions[51].rightp[1] = "LPAR";
	productions[51].rightp[2] = "exps";
	productions[51].rightp[3] = "RPAR";
	productions[52].leftp = "exps";
	productions[52].numofrights = 1;
	productions[52].ifsubp = 0;
	productions[52].subp = "";
	productions[52].rightp[0] = "exp";
	productions[53].leftp = "exps";
	productions[53].numofrights = 3;
	productions[53].ifsubp = 0;
	productions[53].subp = "";
	productions[53].rightp[0] = "exp";
	productions[53].rightp[1] = "COMMA";
	productions[53].rightp[2] = "exps";
	productions[54].leftp = "var";
	productions[54].numofrights = 1;
	productions[54].ifsubp = 1;
	productions[54].subp = "1,@,@,res;";
	productions[54].rightp[0] = "NAME";
	productions[55].leftp = "S'";
	productions[55].numofrights = 1;
	productions[55].ifsubp = 0;
	productions[55].subp = "";
	productions[55].rightp[0] = "program";
	stack<token> tokenstack;//token栈
	stack<int> nstack;//state栈
	stack<string> subpstack;//语义栈
	subpstack.push("#");
	nstack.push(0);
	token ini;
	ini.name = "#";
	ini.type = "#";
	tokenstack.push(ini);
	vector<string> outbuffer;
	outbuffer.push_back("#");
	token now = tokenqueue.front();
	cout << "编译开始！" << endl;
	cout << "词法单元栈" << "	" << "移进规约具体操作" << endl;
	int countsymbol = 0;
	int countint = 0;
	while (!(tokenqueue.empty()))
	{
		printbuffer(outbuffer);
		tableAnalyse(tokenstack, nstack, subpstack, outbuffer, now, GOTO2, countint, countsymbol, counts);
		if (isused)
		{
			tokenqueue.pop();
			//if(!(tokenqueue.empty()))
			now = tokenqueue.front();
		}
		isused = true;
	}
}
