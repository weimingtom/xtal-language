
#include "xtal.h"
#include "xtal_macro.h"
#include "xtal_ch.h"

using namespace xtal;

static void print_usage(){
	fprintf(stderr,
		"usage: xtal [options] [script [args]].\n"
		"Available options are:\n"
		"  -v       show version information\n"
	);
}

static void handle_argv(char** argv){
	int i;
	for(i=1; argv[i]!=0; i++){
		if(argv[i][0]!='-')
			break;

		switch(argv[i][1]){
		case 'v':
			fprintf(stdout, "xtal %d.%d.%d.%d\n", VERSION1, VERSION2, VERSION3, VERSION4);
			break;

		default:
			print_usage();
			return;
		}
	
	}

	if(argv[i]!=0){
		ArrayPtr arg_list(xnew<Array>());
		const char *filename = argv[i++];
		for(i=1; argv[i]!=0; i++){
			arg_list->push_back(argv[i]);
		}
		builtin()->def("argv", arg_list);
		load(filename);
	}
}


#include "xtal_xeg.h"

void debug_line(const SmartPtr<debug::Info>& info){
	//std::cout << Xf("kind=%d, line=%s, file=%s, fun=%s\n")(info->kind(), info->line(), info->file_name(), info->fun_name());

	/*if(info->local_variables()){
		Xfor2(key, value, info->local_variables()->members()){
			std::cout << Xf("key=%s, value=%s\n")(key, value);
		}
	}*/
}

//#include <crtdbg.h>

namespace xtal{ namespace xeg{
	
}
void test_xeg();
}

struct PointSelf : public Base{
	AnyPtr self;

	PointSelf(int n = 0){
		self = SmartPtr<PointSelf>(this);
	}

	virtual void visit_members(Visitor& m){
		Base::visit_members(m);
		m & self;
	}
};

void eoo(){
	int ary[] = {1, 2, 3, 4, 5, 6, 7, 8};
	while(true){
		if((ary[0]*100+ary[1]*10+ary[2]) * ary[3] == ary[4]*1000+ary[5]*100+ary[6]*10+ary[7]){
			printf("%d%d%d*%d= %d%d%d%d, \n", ary[0], ary[1], ary[2], ary[3], ary[4], ary[5], ary[6], ary[7]);
			break;
		}

		std::swap(ary[rand()%8], ary[rand()%8]);
	}
}

const int ww = 6;
const int hh = 6;

void disp(int x, int y){
	static int vs[ww][hh] = {{0}};

	vs[x][y] = 1;

	for(int i=0; i<hh; ++i){
		for(int j=0; j<ww; ++j){
			printf("%s ", vs[j][i] ? "o" : "x");
		}
		printf("\n");
	}

	getchar();
}

void roo(){
	int move[][2] = {
		{-2, -1},
		{-2, 1},
		{-1, -2},
		{-1, 2},
		{2, -1},
		{2, 1},
		{1, -2},
		{1, 2},
	};

	while(true){
		int ary[ww][hh] = {{0}};
		int x = 0;
		int y = 0;

		int ord[ww*hh];

		//printf("a %d\n", ary[2][1]);
		//return;

		for(int i=0; true; ++i){
			ary[x][y] = 1;

			int n = 0;
			int list[8];
			for(int j=0; j<8; ++j){
				//printf("%d %d\n", x+move[j][0], y+move[j][1]);
				if(x+move[j][0]>=0 && x+move[j][0]<ww && y+move[j][1]>=0 && y+move[j][1]<hh
					&& ary[x+move[j][0]][y+move[j][1]]==0){
					list[n] = j;
					n++;
					//printf("eee");
				}
			}

			if(n==0){
				if(i>=ww*hh-1){
					printf("ok\n");
					for(int k=0; k<ww*hh; ++k){
						disp(ord[k]/10, ord[k]%10);
					}
					return;
				}
				else{
					//printf("no %d\n", i);
					break;
				}
			}

			int p = (rand()>>1) % n;
			x = x+move[list[p]][0];
			y = y+move[list[p]][1];

			ord[i] = x*10 + y;
		}
	}
}

struct Piece{
	int shape[3][3];
	int x, y;
};

struct Field{
	int values[9][7];
	Piece pieces[7];

	void disp(){
		for(int y=0; y<9; ++y){
			for(int x=0; x<7; ++x){
				printf("%02d/", values[y][x]);		
			}
			printf("\n");
		}
		getchar();
		//printf("\n");
	}

	friend bool operator <(const Field& a, const Field& b){
		return memcmp(&a.pieces, &b.pieces, sizeof(a.pieces))<0;
	}

	friend bool operator ==(const Field& a, const Field& b){
		return memcmp(&a.pieces, &b.pieces, sizeof(a.pieces))==0;
	}
};

void koo(){
	std::vector<Field> list;
	std::map<Field, int> map;

	Field field = {{
		{ 99, 99,  0,  0, 99, 99, 99},
		{ 99, 99,  0,  0, 99, 99, 99},
		{ 99,  0,  0,  0,  0,  0, 99},
		{  0,  0,  0,  0,  0,  0, 99},
		{  0,  0,  0, 99,  0,  0,  0},
		{  0,  0,  0,  0,  0,  0, 99},
		{ 99,  0,  0,  0,  0,  0, 99},
		{ 99, 99,  0,  0, 99, 99, 99},
		{ 99, 99,  0,  0, 99, 99, 99} 
	},
	{
		{{
			{ 2, 2, },
			{ 2, 2, },
		}, 2, 0, },
		{{
			{ 2, 2, },
			{ 0, 2, },
		}, 1, 5, },
		{{
			{ 2, 2, },
			{ 2, 0, },
		}, 4, 5, },
		{{
			{ 2, 0, },
			{ 2, 2, },
		}, 4, 2, },
		{{
			{ 0, 2, 0 },
			{ 2, 2, 2 },
			{ 0, 2, 0 },
		}, 0, 2, },
		{{
			{ 2, 2, },
		}, 4, 4 },
		{{
			{2},
			{2},
		}, 3, 5 },
	}};

	for(int i=0; i<7; ++i){
		Piece& p = field.pieces[i];
		for(int y=0; y<3; ++y){
			for(int x=0; x<3; ++x){
				if(p.shape[y][x]){
					field.values[p.y+y][p.x+x] = i+1;
				}
			}
		}
	}

	//field.disp();

	list.push_back(field);
	map[field] = 1;

	int move[4][2] = {
		{0, 1},
		{1, 0},
		{0, -1},
		{-1, 0},
	};

	int count = 0;
	while(true){
		int n = (rand()>>1) % 7;
		int m = (rand()>>2) % 4;

		int mx = move[m][0];
		int my = move[m][1];

		Piece& p = field.pieces[n];
		bool ok = true;
		for(int y=0; y<3; ++y){
			for(int x=0; x<3; ++x){
				if(p.shape[y][x]){
					if(p.x+mx+x>=0 && p.x+mx+x<7 && p.y+my+y>=0 && p.y+my+y<9){
						int v = field.values[p.y+my+y][p.x+mx+x]; 
						if(v==0 || v==n+1){
					
						}
						else{
							ok = false;
						}
					}
					else{
						ok = false;
					}
				}
			}
		}

		if(ok){
			for(int y=0; y<3; ++y){
				for(int x=0; x<3; ++x){
					if(p.shape[y][x]){
						field.values[p.y+y][p.x+x] = 0;
					}
				}
			}

			for(int y=0; y<3; ++y){
				for(int x=0; x<3; ++x){
					if(p.shape[y][x]){
						field.values[p.y+my+y][p.x+mx+x] = n+1;
					}
				}
			}

			p.x += mx;
			p.y += my;

			//if((count%100000)==0)field.disp();

			if(1){
				if(int r = map[field]){
					for(int i=r; i<list.size(); ++i){
						map[list[i]] = 0;
					}
					list.resize(r);
				}
				else{
					list.push_back(field);
					map[field] = list.size();
				}
			}

			if(field.pieces[0].x==2 && field.pieces[0].y==7){
				printf("ok %d\n", count);
				printf("ok %d\n", list.size());

				if(list.size()>1000)
					return;

				for(int i=0; i<list.size(); i+=5){
					printf("--%d\n", i);
					list[i].disp();
				}
				return;
			}

			count++;
		}
	}
}

void sol(){

	for(int k=0; k<0x7fffff; ++k){
		k = 108337;
		srand(k);

		/*
		int values[7][7] = {
			{0, 0, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 0, 0},
			{1, 1, 1, 1, 1, 1, 1},
			{1, 1, 1, 2, 1, 1, 1},
			{1, 1, 1, 1, 1, 1, 1},
			{0, 0, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 0, 0},
		};
		/*/

		int values[7][7] = {
			{0, 0, 2, 2, 2, 0, 0},
			{0, 0, 1, 2, 2, 0, 0},
			{1, 1, 1, 1, 2, 2, 2},
			{1, 1, 1, 2, 2, 2, 2},
			{1, 1, 1, 2, 1, 2, 2},
			{0, 0, 1, 1, 1, 0, 0},
			{0, 0, 1, 1, 1, 0, 0},
		};
		//*/

		int move[][2] = {
			{1, 0},
			{0, 1},
			{-1, 0},
			{0, -1},
		};

		while(true){

			int listup[7*7*4];
			int ln = 0;
			int count = 0;
			for(int y=0; y<7; ++y){
				for(int x=0; x<7; ++x){
					if(values[y][x]==1){
						count++;
						for(int m=0; m<4; ++m){
							int mx = move[m][0];
							int my = move[m][1];
							if(x+mx*2>=0 && x+mx*2<7 && y+my*2>=0 && y+my*2<7
								&& values[y+my][x+mx]==1 && values[y+my*2][x+mx*2]==2){
								listup[ln++] = x * 10 + y + m*100;
							}
						}
					}
				}
			}

			if(ln==0){

				if(count==1){
					printf("ok %d\n", k);
					return;
				}

				if((k%1000)==0)
					printf("bad %d\n", k);
				break;
			}

			int v = listup[((rand()>>1) % ln)];
			int x = (v/10)%10;
			int y = v%10;

			int m = v/100;
			int mx = move[m][0];
			int my = move[m][1];
				
			values[y][x] = 2;
			values[y+my][x+mx] = 2;
			values[y+my*2][x+mx*2] = 1;

			//*
			for(int y=0; y<7; ++y){
				for(int x=0; x<7; ++x){
					printf("% 2d", values[y][x]);
				}
				printf("\n");
			}

			getchar();
			//*/
		}
	}
}

int main2(int argc, char** argv){
	//_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF | /*_CRTDBG_CHECK_ALWAYS_DF |*/ _CRTDBG_DELAY_FREE_MEM_DF);
	
	setlocale(LC_ALL, "");

	using namespace xtal::xeg;
	using namespace std;

	try{
		initialize();

		xnew<PointSelf>();
		xnew<PointSelf>(10);

#if 1
		//debug::enable();
		//debug::set_line_hook(fun(&debug_line));
		//debug::set_call_hook(fun(&debug_line));
		//debug::set_return_hook(fun(&debug_line));
			
		
		//load("start.xtal");

		test_xeg();
		/**

		*/




AnyPtr ret = Xsrc
((


));
int n = edit_distance("len", 3, "lem", 3);

		MemoryStreamPtr mm = xnew<MemoryStream>();
		mm->serialize(ret);
		mm->seek(0);
		mm->deserialize()();

		//handle_argv(argv);
		
int c;

		/*		
		c = clock();
		load("../../bench/vec.xtal");
		printf("vec %g\n\n", (clock()-c)/1000.0f);		
		
		c = clock();
		load("../../bench/inst.xtal");
		printf("inst %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		full_gc();
		printf("full_gc %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/loop.xtal");
		printf("loop %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/nested_loops.xtal");
		printf("nested_loops %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/fib.xtal");
		printf("fib %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/loop_iter.xtal");
		printf("loop_iter %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/array_for.xtal");
		printf("array_for %g\n\n", (clock()-c)/1000.0f);

		c = clock();
		load("../../bench/array_each.xtal");
		printf("array_each %g\n\n", (clock()-c)/1000.0f);

		//*/

		//*


		load("../../test/test_empty.xtal");
		load("../../test/test_multiple_values.xtal");
		load("../../test/test_array.xtal");
		//compile_file("../../test/test_float.xtal")->inspect()->p();
		load("../../test/test_float.xtal");
		load("../../test/test_class.xtal");
		load("../../test/test_except.xtal");
		load("../../test/test_fiber.xtal");
		load("../../test/test_fun.xtal");
		//compile_file("../../test/test_iter.xtal")->inspect()->p();
		load("../../test/test_iter.xtal");
		load("../../test/test_fib.xtal");
		load("../../test/test_calc.xtal");
		load("../../test/test_for.xtal");
		load("../../test/test_if.xtal");
		load("../../test/test_nested_loops.xtal");
		load("../../test/test_assign.xtal");
		load("../../test/test_op_assign.xtal");
		load("../../test/test_inc.xtal");
		load("../../test/test_toplevel.xtal");
		load("../../test/test_serialize.xtal");
		
		//*/
#endif

	}catch(AnyPtr e){
		fprintf(stderr, "%s\n", e->to_s()->c_str());
	}

	vmachine()->print_info();
	print_result_of_cache();

	uninitialize();

	return 0;
}

int main(int argc, char** argv){
	int ret = main2(argc, argv);
	return ret;
}
