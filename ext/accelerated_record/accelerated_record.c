#include "accelerated_record.h"

VALUE rb_mAcceleratedRecord;

void
Init_accelerated_record(void)
{
  rb_mAcceleratedRecord = rb_define_module("AcceleratedRecord");
}
