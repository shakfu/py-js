# py for max

An attempt to make a max external for python

## py/pyext docs (for inspiration!)

With the py object you can load python modules and execute the functions therein.
With the pyext you can use python classes to represent full-featured pd/Max message objects.
Multithreading (detached methods) is supported for both objects.
You can send messages to named objects or receive (with pyext) with Python methods.


## TODO

- [ ] add right inlet bang after eval op ends
- [ ] add @run <script>
- [ ] add text edit object
- [ ] if attr has same name as method (the import saga), crash. fixed by making them different
      but there should be another better way.
- [ ] add cythonized access to max c-api..?


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

## Alternative access

- websockets: https://websockets.readthedocs.io/en/stable/intro.html


