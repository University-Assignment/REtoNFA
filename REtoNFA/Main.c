#include <stdio.h>
#include <string.h>

#pragma warning(disable:4996) 

#define MAXNODE 30
#define MAXSTACK 30

struct oneState {
	int a, b, e[2], output;
};

typedef struct _nfaTable {
	int numOfNode;
	struct oneState state[MAXNODE];
} nfaTable;

nfaTable nfaStack[MAXSTACK];
int nfaTop = -1;

void nfaPush(nfaTable m) {
	nfaStack[++nfaTop] = m;
}

nfaTable nfaPop() {
	return nfaStack[nfaTop--];
}

char* insertConcat(char *str) {
	char re_[21];
	int re_len = 0;

	for (int i = 0; *(str + i); i++) {
		re_[re_len++] = *(str + i);
		if (*(str + i + 1))
			if (*(str + i) != '(' && *(str + i + 1) != ')' && *(str + i + 1) != '*' && *(str + i) != '|' && *(str + i + 1) != '|')
				re_[re_len++] = '.';
	}
	re_[re_len] = '\0';
	return re_;
}

int prior(char c) {
	char oper[3] = { '|', '.', '*' };
	for (int i = 1; i <= 3; i++)
		if (oper[i - 1] == c)
			return i;
	return 0;
}

char* convertPostfix(char *str) {
	char post[31];
	char stack[MAXSTACK];
	int top = -1;

	int post_len = 0;
	for (int i = 0; *(str + i); i++) {
		switch (*(str + i)) {
			case 'a':
			case 'b': post[post_len++] = *(str + i); break;
			case '(': stack[++top] = *(str + i); break;
			case ')':
				while (stack[top] != '(')
					post[post_len++] = stack[top--];
				top--;
				break;
			default:
				while (top != -1)
					if (prior(*(str + i)) <= prior(stack[top]))
						post[post_len++] = stack[top--];
					else
						break;
				stack[++top] = *(str + i);
				break;
		}
	}
	while (top != -1)
		post[post_len++] = stack[top--];
	post[post_len] = '\0';
	return post;
}

void init(nfaTable* nt, int a, int b, int e0, int e1, int output) {
	nt->state[nt->numOfNode].a = a;
	nt->state[nt->numOfNode].b = b;
	nt->state[nt->numOfNode].e[0] = e0;
	nt->state[nt->numOfNode].e[1] = e1;
	nt->state[nt->numOfNode].output = output;
	nt->numOfNode++;
}

void plus(struct oneState* os, int value) {
	if (os->a != -1) os->a += value;
	if (os->b != -1) os->b += value;
	if (os->e[0] != -1) os->e[0] += value;
	if (os->e[1] != -1) os->e[1] += value;
}

void symbol(char c) {
	nfaTable nt;
	nt.numOfNode = 0;
	for (int i = 0; i < 2; i++)
		init(&nt, (i == 0 && c == 'a') ? 1 : -1, (i == 0 && c == 'b') ? 1 : -1, -1, -1, i == 0 ? 0 : 1);
	nfaPush(nt);
}

void or() {
	nfaTable last = nfaPop();
	nfaTable first = nfaPop();
	nfaTable list[2] = { first, last };
	nfaTable nt;
	nt.numOfNode = 0;
	init(&nt, -1, -1, 1, first.numOfNode + 1, 0);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < list[i].numOfNode; j++) {
			plus(&list[i].state[j], i == 0 ? 1 : first.numOfNode + 1);
			nt.state[nt.numOfNode] = list[i].state[j];
			if (nt.numOfNode == first.numOfNode || nt.numOfNode == first.numOfNode + last.numOfNode)
				nt.state[nt.numOfNode].e[0] = first.numOfNode + last.numOfNode + 1;
			nt.state[nt.numOfNode].output = 0;
			nt.numOfNode++;
		}
	init(&nt, -1, -1, -1, -1, 1);
	nfaPush(nt);
}

void star() {
	nfaTable first = nfaPop();
	nfaTable nt;
	nt.numOfNode = 0;
	init(&nt, -1, -1, 1, first.numOfNode + 1, 0);
	for (int i = 0; i < first.numOfNode; i++) {
		plus(&first.state[i], 1);
		nt.state[nt.numOfNode] = first.state[i];
		if (i == first.numOfNode - 1) {
			nt.state[nt.numOfNode].e[0] = 1;
			nt.state[nt.numOfNode].e[1] = first.numOfNode + 1;
			nt.state[nt.numOfNode].output = 0;
		}
		nt.numOfNode++;
	}
	init(&nt, -1, -1, -1, -1, 1);
	nfaPush(nt);
}

void concat() {
	nfaTable last = nfaPop();
	nfaTable first = nfaPop();
	nfaTable nt;
	nt.numOfNode = 0;
	for (int i = 0; i < first.numOfNode; i++)
		nt.state[nt.numOfNode++] = first.state[i];
	nt.numOfNode--;
	for (int i = 0; i < last.numOfNode; i++) {
		plus(&last.state[i], first.numOfNode - 1);
		nt.state[nt.numOfNode] = last.state[i];
		if (i == last.numOfNode - 1)
			nt.state[nt.numOfNode].output = 1;
		nt.numOfNode++;
	}
	nfaPush(nt);
}

void constructNFA(char *str) {
	for (int i = 0; *(str + i); i++)
		switch (*(str + i)) {
			case 'a':
			case 'b': symbol(*(str + i)); break;
			case '|': or(); break;
			case '*': star(); break;
			case '.': concat(); break;
		}
}

void ramdaClosure(int* b_flag) {
	int stack[MAXSTACK];
	int top = -1;
	for (int i = 0; i < nfaStack[nfaTop].numOfNode; i++)
		if ((*b_flag & (1 << i)) != 0)
			stack[++top] = i;

	while (top > -1) {
		struct oneState os = nfaStack[nfaTop].state[stack[top--]];
		if (os.a == -1 && os.b == -1)
			for (int i = 0; i < 2; i++)
				if (os.e[i] != -1) {
					*b_flag |= (1 << os.e[i]);
					stack[++top] = os.e[i];
				}
	}
}

void printLine() {
	printf("|");
	for (int i = 0; i < nfaStack[nfaTop].numOfNode - 1; i++)
		printf("---");
	printf("--|\n");
}

void printArr(int b_flag) {
	printLine();
	for (int i = 0; i < nfaStack[nfaTop].numOfNode; i++)
		printf("|%02d", i);
	printf("|\n");
	for (int i = 0; i < nfaStack[nfaTop].numOfNode; i++)
		printf("|%2d", (b_flag & (1 << i)) != 0 ? 1 : 0);
	printf("|\n");
	printLine();
}

int acceptanceTest(char *str) {
	int b_flag = (1 << 0), a_flag;

	ramdaClosure(&b_flag);
	printArr(b_flag);

	for (int i = 0; *(str + i); i++) {
		a_flag = b_flag;
		b_flag = 0;
		for (int j = 0; j < nfaStack[nfaTop].numOfNode; j++) {
			struct oneState os = nfaStack[nfaTop].state[j];
			if ((a_flag & (1 << j)) != 0 && (os.a != -1 || os.b != -1))
				if (os.a != -1 && *(str + i) == 'a')
					b_flag |= (1 << os.a);
				else if (os.b != -1 && *(str + i) == 'b')
					b_flag |= (1 << os.b);
		}
		ramdaClosure(&b_flag);
		printArr(b_flag);
	}
	return (b_flag & (1 << (nfaStack[nfaTop].numOfNode - 1))) != 0;
}

int main() {
	char re[21];
	scanf("%s", re);

	char concat[21];
	strcpy(concat, insertConcat(re));

	char post[21];
	strcpy(post, convertPostfix(concat));

	constructNFA(post);

	printf("%6s %6s %6s %6s %6s %6s\n", "state", "a", "b", "e[0]", "e[1]", "output");
	for (int i = 0; i < nfaStack[nfaTop].numOfNode; i++)
		printf("%6d %6d %6d %6d %6d %6d\n", i, nfaStack[nfaTop].state[i].a, nfaStack[nfaTop].state[i].b, nfaStack[nfaTop].state[i].e[0], nfaStack[nfaTop].state[i].e[1], nfaStack[nfaTop].state[i].output);

	char test[31];
	while (scanf("%s", test))
		printf(acceptanceTest(test) ? "accept\n" : "reject\n");

	return 0;
}