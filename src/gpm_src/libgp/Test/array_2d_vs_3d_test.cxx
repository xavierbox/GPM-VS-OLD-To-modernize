#include "gpm_vbl_array_2d.h"
#include "gpm_basic_defs.h"
#ifdef USE_EIGEN
#include <Eigen/Dense>
#endif
#define BOOST_CHRONO_HEADER_ONLY
#define BOOST_CHRONO_DONT_PROVIDE_HYBRID_ERROR_HANDLING
#include <boost/chrono/chrono_io.hpp>

//#include <ctime>
#include <iostream>
#include <array>
using namespace Slb::Exploration::Gpm;


int array_2d_vs_3d_test(int argc, char *argv[])
{
	typedef std::array<float, 4> fixed_4float_array_type;

	const int x_size=400;
	const int y_size=400;
	const int num_size = 10000;
	gpm_vbl_array_2d<fixed_4float_array_type> f_all(y_size,x_size);
	float_2darray_type f0(y_size,x_size);
	float_2darray_type f1(y_size,x_size,1);
	float_2darray_type f2(y_size,x_size,2);
	float_2darray_type f3(y_size,x_size,3);
	float_2darray_type f4(y_size,x_size,4);

    boost::chrono::steady_clock::time_point start = boost::chrono::steady_clock::now();
	double sum = 0;
	for(int num=0;num < num_size;++num){
		for(int k=0; k < 4;++k){
			for(int i=0; i < y_size;++i){
				for(int j=0; j < x_size;++j){
					float tmp = f0(i,j)*f2(i,j);
					f_all(i,j)[k] += tmp;
				}
			}
		}
	}
    boost::chrono::steady_clock::duration duration = boost::chrono::steady_clock::now() - start;
    boost::chrono::seconds secs = boost::chrono::duration_cast<boost::chrono::seconds>(duration);
    std::cout << "f() took " << secs << " seconds" << sum << "\n";
	start= boost::chrono::steady_clock::now();
	sum=0;
	for(int num=0;num < num_size;++num){
		for(int i=0; i < y_size;++i){
			for(int j=0; j < x_size;++j){
				float tmp = f1(i,j)*f2(i,j);
				f0(i,j) += tmp;
			}
		}
		for(int i=0; i < y_size;++i){
			for(int j=0; j < x_size;++j){
				float tmp = f1(i,j)*f3(i,j);
				f0(i,j) += tmp;
			}
		}
		for(int i=0; i < y_size;++i){
			for(int j=0; j < x_size;++j){
				float tmp = f1(i,j)*f3(i,j);
				f0(i,j) += tmp;
			}
		}
		for(int i=0; i < y_size;++i){
			for(int j=0; j < x_size;++j){
				float tmp = f1(i,j)*f4(i,j);
				f0(i,j) += tmp;
			}
		}
	}


    duration = boost::chrono::steady_clock::now() - start;
    secs = boost::chrono::duration_cast<boost::chrono::seconds>(duration);
	std::cout << "g() took " << secs << " seconds" << sum <<"\n";
    start = boost::chrono::steady_clock::now();
	 sum = 0;
	 for(int num=0;num < num_size;++num){
		 for(int i=0; i < y_size;++i){
			 for(int j=0; j < x_size;++j){
				 fixed_4float_array_type& f_ref(f_all(i,j));
				 for(int k=0; k < 4;++k){
					 float tmp = f0(i,j)*f2(i,j);
					 f_ref[k] += tmp;
				 }
			 }
		 }
	}
     duration = boost::chrono::steady_clock::now() - start;
     secs = boost::chrono::duration_cast<boost::chrono::seconds>(duration);
	std::cout << "h() took " << secs << " seconds" << sum << "\n";
	return 0;
}

#ifdef USE_EIGEN
int array_2d_vs_eigen_test(int argc, char *argv[])
{
	const int x_size=400;
	const int y_size=400;
	const int num_size = 10000;
	Eigen::ArrayXXf fe_0= Eigen::ArrayXXf::Zero(y_size,x_size);
	Eigen::ArrayXXf fe_1= Eigen::ArrayXXf::Constant(y_size,x_size, 1);
	Eigen::ArrayXXf fe_2= Eigen::ArrayXXf::Constant(y_size,x_size, 2);
	Eigen::ArrayXXf fe_3= Eigen::ArrayXXf::Constant(y_size,x_size, 3);
	float_2darray_type f0(y_size,x_size);
	float_2darray_type f1(y_size,x_size,1);
	float_2darray_type f2(y_size,x_size,2);
	float_2darray_type f3(y_size,x_size,3);


	double start = clock();
	double sum = 0;

	for(int num=0;num < num_size;++num){
		for(int i=0; i < y_size;++i){
			for(int j=0; j < x_size;++j){
				float tmp = f1(i,j)*f2(i,j)*f3(i,j);
				f0(i,j) += tmp;
			}
		}
	}

	double current = clock();
	double duration = (double)(current - start) / CLOCKS_PER_SEC;
	std::cout << "gpm_array() took " << duration << " seconds" << sum <<"\n";

	start = clock();
	sum = 0;
	for(int num=0;num < num_size;++num){
		fe_0 += fe_1*fe_2*fe_3;
	}
	current = clock();
	duration = (double)(current - start) / CLOCKS_PER_SEC;
	std::cout << "eigen() took " << duration << " seconds" << sum << "\n";

	for(int i=0; i < y_size;++i){
		for(int j=0; j < x_size;++j){
			float diff = std::abs(f0(i,j) -fe_0(i,j));
			if (diff > 1e-12){
				std::cout << "Diff > 1e-6 " << diff << "\n";
			}
		}
	}
	return 0;
}

#endif
