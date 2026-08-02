#include "tst.hpp"
#include "tst.hpp"

bool included_from_another_translation_unit()
{
    tst::check(true);
    return true;
}
