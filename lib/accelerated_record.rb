require "accelerated_record/version"
require "accelerated_record/accelerated_record"

module AcceleratedRecord
  # Your code goes here...
  def self.speedup!
    ActiveRecord::Associations::Preloader::Association.class_eval do
      # def run(preloader)
      #   records = load_records do |record|
      #     owner = owners_by_key[convert_key(record[association_key_name])]
      #     association = owner.association(reflection.name)
      #     association.set_inverse_instance(record)
      #   end
      #
      #   owners.each do |owner|
      #     associate_records_to_owner(owner, records[convert_key(owner[owner_key_name])] || [])
      #   end
      # end

      # The name of the key on the associated records
      # def association_key_name
      #   @association_key_name ||= reflection.join_primary_key(klass)
      # end
    end

    ActiveRecord::Reflection::BelongsToReflection.class_eval do
      def association_class
        if polymorphic?
          ::ActiveRecord::Associations::BelongsToPolymorphicAssociation
        else
          ::AcceleratedRecord::BelongsToAssociation
        end
      end
    end
  end
end
