#include "accelerated_record.h"

VALUE rb_mAcceleratedRecord;
VALUE rb_cBelongsToAssociation;

typedef struct {
  VALUE owner;
  VALUE reflection;
} belongs_to_association_t;

void belongs_to_association_mark(void* uncasted_data)
{
  belongs_to_association_t* data = uncasted_data;
	rb_gc_mark(data->owner);
	rb_gc_mark(data->reflection);
}

void belongs_to_association_free(void* data)
{
	free(data);
}

size_t belongs_to_association_size(const void* data)
{
	return sizeof(belongs_to_association_t);
}

static const rb_data_type_t belongs_to_association_type = {
  .wrap_struct_name = "belongs_to_association_t",
  .function = {
    .dmark = belongs_to_association_mark,
    .dfree = belongs_to_association_free,
    .dsize = belongs_to_association_size,
  },
  .data = NULL,
  .flags = RUBY_TYPED_FREE_IMMEDIATELY,
};

VALUE belongs_to_association_alloc(VALUE self)
{
	belongs_to_association_t* data = malloc(sizeof(belongs_to_association_t));
	return TypedData_Wrap_Struct(self, &belongs_to_association_type, data);
}

VALUE belongs_to_association_initialize(VALUE self, VALUE owner, VALUE reflection)
{
	belongs_to_association_t* data;
	TypedData_Get_Struct(self, belongs_to_association_t, &belongs_to_association_type, data);

	data->owner = owner;
  data->reflection = reflection;

	return self;
}

void Init_accelerated_record(void)
{
  rb_mAcceleratedRecord = rb_define_module("AcceleratedRecord");

  rb_cBelongsToAssociation = rb_define_class_under(rb_mAcceleratedRecord, "BelongsToAssociation", rb_cObject);
  rb_define_alloc_func(rb_cBelongsToAssociation, belongs_to_association_alloc);
	rb_define_method(rb_cBelongsToAssociation, "initialize", belongs_to_association_initialize, 2);
}
