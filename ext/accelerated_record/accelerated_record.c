#include "accelerated_record.h"

VALUE rb_mAcceleratedRecord, rb_mExt, rb_cBelongsToAssociation, rb_mResult;

typedef struct {
  VALUE owner;
  VALUE reflection;
  VALUE loaded;
  VALUE target;
} belongs_to_association_t;

void belongs_to_association_mark(void* uncasted_data)
{
  belongs_to_association_t* data = uncasted_data;
  rb_gc_mark(data->owner);
  rb_gc_mark(data->reflection);
  // Loaded is either going to be `Qtrue` or `Qfalse` so no need to mark.
  rb_gc_mark(data->target);
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

inline belongs_to_association_t* get_belongs_to_association(VALUE self)
{
  belongs_to_association_t* data;
  TypedData_Get_Struct(self, belongs_to_association_t, &belongs_to_association_type, data);
  return data;
}

VALUE belongs_to_association_initialize(VALUE self, VALUE owner, VALUE reflection)
{
  belongs_to_association_t* data = get_belongs_to_association(self);
  data->owner = owner;
  data->reflection = reflection;
  data->loaded = Qfalse;
  data->target = Qnil;
  return self;
}

VALUE belongs_to_association_klass(VALUE self)
{
  belongs_to_association_t* data = get_belongs_to_association(self);
  return rb_funcallv_public(data->reflection, rb_intern("klass"), 0, NULL);
}

VALUE belongs_to_association_loaded(VALUE self)
{
  belongs_to_association_t* data = get_belongs_to_association(self);
  return data->loaded;
}

//   # Asserts the \target has been loaded setting the \loaded flag to +true+.
//   def loaded!
//     @loaded = true
//     @stale_state = stale_state
//     @inversed = false
//   end
VALUE belongs_to_association_set_loaded(VALUE self)
{
  belongs_to_association_t* data = get_belongs_to_association(self);
  data->loaded = Qtrue;
  return Qnil;
}

//   def inversed_from(record)
//     replace_keys(record)
//     super
//   end
//
//   def inversed_from(record)
//     self.target = record
//     @inversed = !!record
//   end
//   alias :inversed_from_queries :inversed_from
//
//   def replace_keys(record)
//     owner[reflection.foreign_key] = record ? record._read_attribute(primary_key(record.class)) : nil
//   end
VALUE belongs_to_association_inversed_from(VALUE self, VALUE record)
{
  belongs_to_association_t* data = get_belongs_to_association(self);
  // FIXME: Call/implement `replace_keys`.
  data->target = record;
  // FIXME Set `@inversed`.
  return Qtrue;
}

VALUE belongs_to_association_reflection(VALUE self)
{
  belongs_to_association_t* data = get_belongs_to_association(self);
  return data->reflection;
}

// The below method pretty much implements all the following in one go:
//
//   # Set the inverse association, if possible
//   def set_inverse_instance(record)
//     if inverse = inverse_association_for(record)
//       inverse.inversed_from(owner)
//     end
//     record
//   end
//
//   def inverse_association_for(record)
//     if invertible_for?(record)
//       record.association(inverse_reflection_for(record).name)
//     end
//   end
//
//   # Can be redefined by subclasses, notably polymorphic belongs_to
//   # The record parameter is necessary to support polymorphic inverses as we must check for
//   # the association in the specific class of the record.
//   def inverse_reflection_for(record)
//     reflection.inverse_of
//   end
//
//   # NOTE - for now, we're only supporting inverse setting from belongs_to back onto
//   # has_one associations.
//   def invertible_for?(record)
//     inverse = inverse_reflection_for(record)
//     inverse && inverse.has_one?
//   end
VALUE belongs_to_association_set_inverse_instance(VALUE self, VALUE record)
{
  belongs_to_association_t* data = get_belongs_to_association(self);

  VALUE inverse_reflection = rb_funcallv_public(data->reflection, rb_intern("inverse_of"), 0, NULL);
  if (NIL_P(inverse_reflection)) { goto end; }

  // Early return unless `inverse_reflection.has_one?`.
  VALUE has_one = rb_funcallv_public(inverse_reflection, rb_intern("has_one?"), 0, NULL);
  if (has_one != Qtrue) { goto end; }

  VALUE inverse_reflection_name = rb_funcallv_public(inverse_reflection, rb_intern("name"), 0, NULL);
  // Effectively doing `inverse_association = record.association(inverse_reflection_name)`.
  VALUE inverse_association = rb_funcallv_public(record, rb_intern("association"), 1, &inverse_reflection_name);
  rb_funcallv_public(inverse_association, rb_intern("inversed_from"), 1, &data->owner);

end:
  return record;
}

//   # Sets the target of this association to <tt>\target</tt>, and the \loaded flag to +true+.
//   def target=(target)
//     @target = target
//     loaded!
//   end
VALUE belongs_to_association_set_target(VALUE self, VALUE target)
{
  belongs_to_association_t* data = get_belongs_to_association(self);
  data->target = target;
  return belongs_to_association_set_loaded(self);
}

VALUE result_hash_rows(VALUE self)
{
  ID ivHash_rows = rb_intern("@hash_rows");

  VALUE hash_rows = rb_ivar_get(self, ivHash_rows);
  if (!NIL_P(hash_rows)) {
    return hash_rows;
  }

  VALUE columns = rb_ivar_get(self, rb_intern("@columns"));
  long columns_length = RARRAY_LEN(columns);

  VALUE *frozen_columns = malloc(sizeof(VALUE) * columns_length);
  for (long index = 0; index < columns_length; index++) {
    VALUE column = RARRAY_AREF(columns, index);
    frozen_columns[index] = rb_str_dup_frozen(column);
  }

  VALUE rows = rb_ivar_get(self, rb_intern("@rows"));
  long rows_length = RARRAY_LEN(rows);

  VALUE hashed_rows = rb_ary_new2(rows_length);
  for (long row_index = 0; row_index < rows_length; row_index++) {
    VALUE row = RARRAY_AREF(rows, row_index);

    VALUE hashed_row = rb_hash_new();
    for (long column_index = 0; column_index < columns_length; column_index++) {
      VALUE column_name = frozen_columns[column_index];
      VALUE column_value = RARRAY_AREF(row, column_index);
      rb_hash_aset(hashed_row, column_name, column_value);
    }

    rb_ary_push(hashed_rows, hashed_row);
  }

  free(frozen_columns);
  rb_ivar_set(self, ivHash_rows, hashed_rows);
  return hashed_rows;
}

void Init_accelerated_record(void)
{
  rb_mAcceleratedRecord = rb_define_module("AcceleratedRecord");
  rb_mExt = rb_define_module_under(rb_mAcceleratedRecord, "Ext");

  rb_cBelongsToAssociation = rb_define_class_under(rb_mExt, "BelongsToAssociation", rb_cData);
  rb_define_alloc_func(rb_cBelongsToAssociation, belongs_to_association_alloc);
  rb_define_method(rb_cBelongsToAssociation, "initialize", belongs_to_association_initialize, 2);
  rb_define_method(rb_cBelongsToAssociation, "klass", belongs_to_association_klass, 0);
  rb_define_method(rb_cBelongsToAssociation, "loaded?", belongs_to_association_loaded, 0);
  rb_define_method(rb_cBelongsToAssociation, "loaded!", belongs_to_association_set_loaded, 0);
  rb_define_method(rb_cBelongsToAssociation, "inversed_from", belongs_to_association_inversed_from, 1);
  rb_define_method(rb_cBelongsToAssociation, "reflection", belongs_to_association_reflection, 0);
  rb_define_method(rb_cBelongsToAssociation, "set_inverse_instance", belongs_to_association_set_inverse_instance, 1);
  rb_define_method(rb_cBelongsToAssociation, "target=", belongs_to_association_set_target, 1);

  rb_mResult = rb_define_module_under(rb_mExt, "Result");
  rb_define_method(rb_mResult, "hash_rows", result_hash_rows, 0);
}
