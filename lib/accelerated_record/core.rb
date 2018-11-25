require 'accelerated_record/version'

require 'accelerated_record/accelerated_record'

module AcceleratedRecord
  def self.speedup!
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
