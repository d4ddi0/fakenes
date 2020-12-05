require 'rake/loaders/makefile'

@cc = 'cc'
@linker = 'cc'
@makedepend = '/usr/X11R6/bin/makedepend'
@frameworkPaths = %w(~/Library/Frameworks /Library/Frameworks)
@cflags = '-arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk '
@includes = ''
@defines = ''
@ldflags = '-arch ppc -arch i386 -isysroot /Developer/SDKs/MacOSX10.4u.sdk '
@libs = ''
@systemFrameworks = []
@frameworks = []
@cleanfiles = []

###########
# Cleanup #
###########

task(:clean) do |task|
	rm_rf(@cleanfiles)
	rm_rf(BUNDLEDIR)
end

################
# Installation #
################

def installRule(dir, file)
	target = File::join(dir, File::basename(file))
	file(target => [dir, file]) do |t|
		cp(file, target)
	end
end

def installTask(taskName, dir, files)
	if files.is_a?(String)
		files = [files]
	end

	array = []
	for f in files do
		array << File::join(dir, File::basename(f))
		installRule(dir, f)
	end
	task(taskName => array)
end

def installTaskRecursive(taskName, dest, src)
	target = File::join(dest, File::basename(src))
	file(target => [dest, src]) do |t|
		rm_rf(t.name)
		cp_r(t.prerequisites[1], t.prerequisites[0])
	end
	task(taskName => target)
end

def bundleDir(dir)
	file(dir) do |t|
		sh("/Developer/Tools/SetFile -a B #{dir}")
	end
end

directory(BUNDLEDIR = "#{NAME}.app")
directory(CONTENTSDIR = "#{BUNDLEDIR}/Contents")
directory(BINDIR = "#{CONTENTSDIR}/MacOS")
directory(RESOURCEDIR = "#{CONTENTSDIR}/Resources")
directory(FRAMEWORKDIR = "#{CONTENTSDIR}/Frameworks")

bundleDir(BUNDLEDIR)

##############
# Frameworks #
##############

def installFrameworks(task)
	for name in @frameworks
		framework = nil
		for path in @frameworkPaths
			try = "#{File::expand_path(path)}/#{name}.framework"
			if File::directory?(try)
				framework = try
				break
			end
		end

		@libs += " -framework #{name} "
		@includes += " -I#{framework}/Headers "

		installTaskRecursive(task, FRAMEWORKDIR, framework)
	end

	for name in @systemFrameworks
		@libs += " -framework #{name} "
	end

	for dir in @frameworkPaths
		@cflags += " -F#{File::expand_path(dir)} "
		@ldflags += " -F#{File::expand_path(dir)} "
	end
end

###############
# Compilation #
###############

def buildBinary(task, path, file, sources)
	objects = []
	target = "#{path}/#{file}"
	for source in sources
		object = "#{File::dirname(source)}/#{File::basename(source, '.*')}.o"
		file(object => source) do |task|
			sh("#{@cc} #{@cflags} #{@includes} #{@defines} -o \"#{task.name}\" -c #{task.prerequisites[0]}")
		end
		objects.push(object)
		@cleanfiles.push(object)

		depfile = "#{File::dirname(source)}/.#{File::basename(source, '.*')}.dep.mf"
		file(depfile => source) do |task|
			sh("#{@makedepend} -f- -- #{@includes} #{@defines} -- #{task.prerequisites} > #{task.name} 2> /dev/null")
		end
		import depfile
		@cleanfiles.push(depfile)
	end

	file(target => [path, *objects]) do |task|
		sh("#{@linker} #{@ldflags} -o \"#{task.name}\" #{task.prerequisites[1..-1].join(' ')} #{@libs}")
	end

	task(task => target)
end
