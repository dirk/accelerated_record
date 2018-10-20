require "bundler/gem_tasks"
require "rspec/core/rake_task"

RSpec::Core::RakeTask.new(:spec)

require "rake/extensiontask"

task :build => :compile

Rake::ExtensionTask.new("accelerated_record") do |ext|
  ext.lib_dir = "lib/accelerated_record"
end

task :default => [:clobber, :compile, :spec]
