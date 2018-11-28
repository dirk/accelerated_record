module AcceleratedRecord
  module Accelerators
    module Result
      def self.accelerate!
        ActiveRecord::Result.prepend AcceleratedRecord::Ext::Result
      end
    end
  end
end
