#include <iostream>
#include <queue>
#include <string>
#include <algorithm>
#include <utility>

using namespace std;

priority_queue<pair<int, int>, vector<pair<int, int>>, greater<pair<int, int>>> pq;

int main()
{
	for (int i = 1; i <= 100; ++i) {
		pq.push(make_pair(i*(i + 1), i + 1));
	}
	int N; cin >> N;
	int count = 0;
	pair<int, int> res;
	while (count < N) {
		res = pq.top();
		pq.push(make_pair(res.first * (res.second + 1), res.second + 1));
		pq.pop();
		pair<int, int> temp;
		while ((temp = pq.top()).first == res.first) {
			pq.push(make_pair(temp.first * (temp.second + 1), temp.second + 1));
			pq.pop();
		}
		++count;
	}
	cout << res.first << endl;
	return 0;
}


