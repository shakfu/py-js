# py for max

An attempt to make a max external for python


## Building

place the following script in source folder of max-sdk-8.0.3

```ruby
#!/usr/bin/env ruby -wKU
# encoding: utf-8

puts "cleanup"
system "rm -rf ../externals/py.mxo && rm -rf basics/py/build && xattr -cr ./basics/py"

sdk_examples_dir = "."
Dir.chdir sdk_examples_dir
sdk_examples_dir = Dir.pwd

def build_projects_for_dir(path)
  puts
  puts "Building Projects for Directory: #{path}"
  
  Dir.foreach path do |filename|

    puts "scanning: #{filename}"

    if filename.match(/.*\.xcodeproj/)
      puts "  Building #{filename}" 
      result = `cd "#{path}"; xcodebuild -project #{filename} 2>&1`
      puts result
      if result.match(/\*\* BUILD SUCCEEDED \*\*/)
        puts "    (success)"
      else
        puts "    (FAIL) ************************************"
        #puts result
      end
    end

  end
  
end

build_projects_for_dir("basics/py")

puts "You now have a Max Package that you can use."
```
