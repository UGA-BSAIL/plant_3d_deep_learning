
#include "Bbox.h"
#include <string>




BBox::BBox()
    :bbox_translation(0.0f, 0.0f, 0.0f), bbox_rotation(0.0f, 0.0f, 0.0f), bbox_scale(1.0f, 1.0f, 1.0f)
{
    std::string phrase = "Enter label name";
    strcpy_s(labelname, phrase.size() + 1, phrase.c_str());

 //   strcpy_s(labelname ,10,"Enter label name");
}
