// Array type concepts
#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <iterator>


// This will need all the C scoping and potential packing to get it right
// But this is the idea
// Pack all things into arrays, and use the string struct to hold the data
// Indprop will need to generate and keep the strings somehow
// Also need to fix the size of the ints to either 32 or 64 bit
// Same with the enum

// These are for the handshakes with P/Invoke

#ifdef __cplusplus
extern "C" {
#endif

enum GPM_ACF_POD_TYPES { INTEGER = 0, FLOAT = 1, STRING = 2, DOUBLE = 3 };

typedef unsigned long gpm_write_size_t;
typedef long gpm_write_stride_t;

#pragma pack(push, 8)
struct gpm_string_holder {
	const char* ptr;
	gpm_write_size_t num;
};
/* used for arrays (1d and 2d) and scalars */
struct gpm_base_memory_2d_layout {
	gpm_write_size_t num_rows;
	gpm_write_size_t num_cols;
	gpm_write_stride_t row_stride;
	gpm_write_stride_t col_stride;
	/*
	 *Points to the linear memory
	 Can be int*, float*, double* and string_holder*
    */
	void* mem_ptr;
	/*GPM_ACF_POD_TYPES*/ int mem_type;
};

/* Used for arrays */
struct gpm_indirection_memory_2d_layout {
	gpm_write_size_t num;
	/* Point to an array of base_memory_2d_layouts
	 Which again needs to point to linear arrays of some sort
    */
	gpm_base_memory_2d_layout* mem_ptr;
};

#pragma pack(pop)

void marshall_int_data(gpm_base_memory_2d_layout block)
{
	// Should base the cast on 
	int* base = (int*)(block.mem_ptr);
	for (gpm_write_size_t i = 0; i < block.num_rows; ++i) {
		for (gpm_write_size_t j = 0; j < block.num_cols; ++j) {
			std::cout << base[i * block.row_stride + j * block.col_stride] << " ";
		}
		std::cout << std::endl;
	}

}
void marshall_float_data(gpm_base_memory_2d_layout block)
{
	// Should base the cast on 
	float* base = (float*)(block.mem_ptr);
	for (gpm_write_size_t i = 0; i < block.num_rows; ++i) {
		for (gpm_write_size_t j = 0; j < block.num_cols; ++j) {
			std::cout << base[i * block.row_stride + j * block.col_stride] << " ";
		}
		std::cout << std::endl;
	}

}
void marshall_double_data(gpm_base_memory_2d_layout block)
{
	// Should base the cast on 
	double* base = (double*)(block.mem_ptr);
	for (gpm_write_size_t i = 0; i < block.num_rows; ++i) {
		for (gpm_write_size_t j = 0; j < block.num_cols; ++j) {
			std::cout << base[i * block.row_stride + j * block.col_stride] << " ";
		}
		std::cout << std::endl;
	}

}
void marshall_string_data(gpm_base_memory_2d_layout block)
{
	// Should base the cast on 
	gpm_string_holder* base = (gpm_string_holder*)(block.mem_ptr);
	for (gpm_write_size_t i = 0; i < block.num_rows; ++i) {
		for (gpm_write_size_t j = 0; j < block.num_cols; ++j) {
			gpm_string_holder str = base[i * block.row_stride + j * block.col_stride];
			std::cout << std::string(str.ptr) << ":"<<str.num << " ";
		}
		std::cout << std::endl;
	}

}
void marshal_the_data(gpm_indirection_memory_2d_layout* arr, gpm_write_size_t num)
{
	for (gpm_write_size_t it_num = 0; it_num < num; ++it_num) {
		gpm_indirection_memory_2d_layout it = arr[it_num];
		for (gpm_write_size_t l = 0; l < it.num; ++l) {
			const auto layout = it.mem_ptr[l];
            // Should base the cast on 
            switch(layout.mem_type) {
			case INTEGER:
				marshall_int_data(layout);
				break;
            case DOUBLE:
                marshall_double_data(layout);
                break;
            case FLOAT:
                marshall_float_data(layout);
                break;
			case STRING:
				marshall_string_data(layout);
				break;
			default:
				break;
			}
		}
	}
}

#ifdef __cplusplus
}
#endif

// The rest for C++ will need helper classes to smooth things out, like casting to the type

typedef std::vector< gpm_indirection_memory_2d_layout> memory_mapper_type;

int test_memptr_wrapper(int argc, char* argv[]) {
	// These need to be in scope as long as the 
// for a scalar
	{
		int a_sclar = 1;
		gpm_base_memory_2d_layout holder{};
		holder.num_rows = 1; holder.num_cols = 1;
		holder.row_stride = 0; holder.col_stride = 0;
		holder.mem_ptr = &a_sclar;
		holder.mem_type = INTEGER;
		gpm_indirection_memory_2d_layout scal{};
		scal.num = 1;
		scal.mem_ptr = &holder;
	}
// For a vector
	{
		std::vector<int> a_vec(10);
		gpm_base_memory_2d_layout holder{};
		holder.num_rows = 1; holder.num_cols = 10;
		holder.row_stride = 0; holder.col_stride = 1;
		holder.mem_ptr = a_vec.data();
		gpm_indirection_memory_2d_layout scal{};
		scal.num = 1;
		scal.mem_ptr = &holder;
	}
    // For a vector of vectors
    {
        std::vector<std::vector<int>> a_vec(8, std::vector<int>(10));
        std::vector<gpm_base_memory_2d_layout> holder;
        int count = 0;
        for (auto& it : a_vec) {
            std::fill(it.begin(), it.end(), count);
            gpm_base_memory_2d_layout a_hold{};
            a_hold.num_rows = 1;
            a_hold.num_cols = it.size();
            a_hold.row_stride = 0;
            a_hold.col_stride = 1;
            a_hold.mem_ptr = it.data();
            holder.push_back(a_hold);
            ++count;
        }
        gpm_indirection_memory_2d_layout scal;
        scal.num = holder.size();
        scal.mem_ptr = holder.data();

		std::vector< gpm_indirection_memory_2d_layout> parm_holder;
        // Pack up all the parm parts
        // Thsi would be index equal to the other arrays in real use
		parm_holder.push_back(scal);
        // Now lets traverse the thing
		for (auto& it : parm_holder) {
			for (int l = 0; l < it.num; ++l) {
				const auto layout = it.mem_ptr[l];
				int* base = static_cast<int*>(layout.mem_ptr);
				for (int i = 0; i < layout.num_rows; ++i) {
					for (int j = 0; j < layout.num_cols; ++j) {
						std::cout << base[i * layout.row_stride + j * layout.col_stride] << " ";
					}
					std::cout << std::endl;
				}
			}
		}
		std::cout << "C style" << std::endl;
		marshal_the_data(parm_holder.data(), parm_holder.size());
    }

	// For a vector of vectors of strings
	{
		std::vector<std::vector<std::string>> a_vec(8, std::vector<std::string>(10));
		std::vector<std::vector<gpm_string_holder>> a_str_holder(8);
		std::vector<gpm_base_memory_2d_layout> holder;
		int count = 0;
		for (auto& it : a_vec) {
			std::fill(it.begin(), it.end(), std::to_string(count));
			std::transform(it.begin(), it.end(), std::back_inserter(a_str_holder[count]), [](const std::string & str)
				{
					return gpm_string_holder{ str.c_str(), static_cast<gpm_write_size_t>(str.size()) };
				});
			gpm_base_memory_2d_layout a_hold{};
			a_hold.num_rows = 1;
			a_hold.num_cols = it.size();
			a_hold.row_stride = 0;
			a_hold.col_stride = 1;
			a_hold.mem_ptr = a_str_holder[count].data();
			a_hold.mem_type = STRING;
			holder.push_back(a_hold);
			++count;
		}
		gpm_indirection_memory_2d_layout scal;
		scal.num = holder.size();
		scal.mem_ptr = holder.data();
		std::vector< gpm_indirection_memory_2d_layout> parm_holder;
		// Pack up all the parm parts
		// Thsi would be index equal to the other arrays in real use
		parm_holder.push_back(scal);

		marshal_the_data(parm_holder.data(), parm_holder.size());
	}
    return 0;
}