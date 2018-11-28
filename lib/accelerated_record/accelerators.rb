require 'accelerated_record/accelerators/reflection/belongs_to_reflection'
require 'accelerated_record/accelerators/result'

module AcceleratedRecord
  module Accelerators
    def self.all
      [
        Reflection::BelongsToReflection,
        Result,
      ]
    end
  end
end
