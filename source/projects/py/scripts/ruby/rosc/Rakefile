require 'rake/testtask'
require 'rake/rdoctask'

task :default => :rdoc
Rake::RDocTask.new do |rd|
  rd.rdoc_files.add ['README','AUTHORS','TODO','lib/**/*.rb']
  #rd.template = ENV['HOME']+'/src/allison/allison.rb'
  rd.rdoc_dir = 'doc'
  rd.options = ['-x_darcs','-xtest']
  rd.title = 'rosc'
  rd.options += ['--line-numbers','--inline-source']
end

Rake::TestTask.new do |t|
  #t.verbose = true
end

# vim: filetype=ruby
