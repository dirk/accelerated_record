module AcceleratedRecord
  module Accelerators
    module Reflection
      module BelongsToReflection
        def self.accelerate!
          ActiveRecord::Reflection::BelongsToReflection.class_eval do
            def association_class
              if polymorphic?
                ::ActiveRecord::Associations::BelongsToPolymorphicAssociation
              else
                ::AcceleratedRecord::Ext::BelongsToAssociation
              end
            end
          end
        end
      end
    end
  end
end
