require 'accelerated_record/version'

# Load the native extension before anything else.
require 'accelerated_record/accelerated_record'

require 'accelerated_record/accelerators'

module AcceleratedRecord
  def self.accelerate!
    Accelerators.all.each(&:accelerate!)
  end
end
