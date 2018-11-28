require 'accelerated_record/accelerators/reflection/belongs_to_reflection'

module AcceleratedRecord
  module Accelerators
    def self.all
      [
        Reflection::BelongsToReflection,
      ]
    end
  end
end
