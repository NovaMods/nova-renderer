#include "ivertex_buffer.h"

void ivertex_buffer::set_data(std::vector<float> data, ivertex_buffer::format data_format,
                              ivertex_buffer::usage data_usage) {
    this->data_format = data_format;
}

format ivertex_buffer::get_format() {
    return data_format;
}



