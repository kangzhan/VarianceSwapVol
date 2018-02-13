#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <cmath>
#include <string>
#include <cstdlib>

using namespace std;
const double i = 0.034788;//根据短期国债收益率取无风险利率
const double r = log(1 + i);//计算连续利息力
const int days = 365;//定义一年的天数

//定义期权类
class option {
public:
	double price;//期权价格
	double E;//期权执行价格
	int T;//期权到期日
	char type;//期权的类型（看涨或者看跌）
	double S0;//期权标的的价格
	bool operator <(option a)//重载运算符“<”，便于之后期权的积分
	{
		return(this->E < a.E);
	}
};

//将所有的到期日T整合到一个vector里面
vector<int> classify(vector<option> data)
{
	vector<int> duetime;
	vector<int>::iterator ret;
	for (unsigned int i = 0; i < data.size(); i++)
	{
		ret = find(duetime.begin(), duetime.end(), data[i].T);
		if (ret == duetime.end())//如果ret返回vector的end指针，说明没找到相应的T，则将该T加入向量中
			duetime.push_back(data[i].T);
	}
	sort(duetime.begin(), duetime.end());
	return duetime;
}

//按照T分类计算Evar
vector<pair<int, double>> calculate(vector<int> duetime, vector<option> data)
{
	vector<option> cop;//看涨期权类向量
	vector<option> pop;//看跌期权类向量
	vector<int>::iterator ret;//int类向量迭代器
	vector<option>::iterator umt;//期权类向量迭代器
	double F0;
	double Evar, Evar1, Evar2;
	double S0 = data[0].S0;
	pair<int, double> result;//<T,Evar>组合成一个pair
	vector<pair<int, double>> final;//pair类向量
	vector<pair<int, double>>::iterator frt;//pair类向量迭代器
	for (ret = duetime.begin(); ret != duetime.end(); ++ret)
	{
		Evar1 = 0;
		Evar2 = 0;
		F0 = S0*exp(r*(*ret) / days);//计算F0
		for (unsigned int i = 0; i < data.size(); i++)
		{
			if (data[i].T == *ret&&data[i].type == 'P'&&data[i].E <= F0)//读取满足积分条件的看跌期权数据
				pop.push_back(data[i]);
			if (data[i].T == *ret&&data[i].type == 'C'&&data[i].E >= F0)//读取满足积分条件的看涨期权数据
				cop.push_back(data[i]);
			sort(pop.begin(), pop.end());//用重载的运算符对期权类向量进行排序，以便于积分
			sort(cop.begin(), cop.end());//用重载的运算符对期权类向量进行排序，以便于积分
		}
		for (umt = pop.begin(); umt != pop.end(); ++umt)//计算积分的第一部分
		{
			if (umt + 1 != pop.end())
				Evar1 = Evar1 + ((*(umt + 1)).E- (*umt).E )*((*umt).price / pow((*umt).E, 2) + (*(umt + 1)).price / pow((*(umt + 1)).E, 2)) / 2;
		}
		for (umt = cop.begin(); umt != cop.end(); ++umt)//计算积分的第二部分
		{
			if (umt + 1 != cop.end())
				Evar2 = Evar2 + ((*(umt + 1)).E - (*umt).E)*((*umt).price / pow((*umt).E, 2) + (*(umt + 1)).price / pow((*(umt + 1)).E, 2)) / 2;
		}
		Evar = (Evar1 + Evar2) * 2 * exp(r*(*ret) / days) / (*ret);//得到Evar
		result = make_pair(*ret, Evar);//将<T,Evar>组合成pair
		final.push_back(result);//将pair保存在pair类向量result里面
		cop.swap(vector<option>());
		pop.swap(vector<option>());
	}
	for (frt = final.begin(); frt != final.end(); ++frt)//输出结果
		cout << "T=" << (*frt).first << ',' << "Evar=" << (*frt).second << endl;
	return final;//返回pair类向量
}

int main()
{
	vector<option> data;
	vector<int> duetime;
	option etf;
	vector<pair<int, double>> final;
	vector<pair<int, double>>::iterator frt;
	FILE *fp;
	FILE *filename;
	char fileway[256];
	filename = fopen("filename.txt","r");//打开文件filename.txt读取数据路径
	while (fscanf(filename, "%s", fileway) != EOF)
	{
		cout << "Time:";
		for (int i = 6; i < 14; i++)
			cout << *(fileway + i);
		cout << endl;
		fp = fopen(fileway, "r");//打开数据文件
		while (1) {
			fscanf(fp, "%lf,%lf,%d,%c,%lf", &etf.price, &etf.E, &etf.T, &etf.type, &etf.S0);//读取数据
			data.push_back(etf);
			if (feof(fp)) break;
		}
		fclose(fp);
		duetime = classify(data);//获得所有的T
		final = calculate(duetime, data);//按照T分类计算所有的Evar
		data.swap(vector<option>());//清空期权类容器
		string InitialFileName("results\\results" + to_string(201710) + fileway[12] + fileway[13] + ".csv");
		ofstream outfile(InitialFileName, ios::trunc);//输出final到csv文件
		outfile << "T" << "," << "Evar" << endl;
		for (frt = final.begin(); frt != final.end(); ++frt)
			outfile << (*frt).first << "," << (*frt).second << endl;
		outfile.close();
	}
	system("pause");
	return 0;
}