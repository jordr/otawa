/*
 *	$Id$
 *	Copyright (c) 2003, IRIT UPS.
 *
 *	manager.cc -- manager classes implementation.
 */

#include <otawa/platform.h>
#include <otawa/manager.h>

namespace otawa {


/**
 * @class LoadException
 * Exception thrown when a loader encounters an error during load.
 */

/**
 * Build a load exception with a simple message.
 * @param message	Exception message.
 */
LoadException::LoadException(const String& message): Exception(message) {
}

/**
 * Build a load exception with a formatted message.
 * @param format	Format of the message.
 * @param args		Arguments of the format.
 */
LoadException::LoadException(const char *format, va_list args): Exception(format, args) {
}


/**
 * Build a load exception with a formatted message.
 * @param format	Format of the message.
 * @param ...		Arguments of the format.
 */
LoadException::LoadException(const char *format, ...) {
	va_list args;
	va_start(args, format);
	build(format, args);
	va_end(args);
}


/**
 * @class UnsupportedPlatformException
 * Exception thrown when a loader cannot handle a platform.
 */

/**
 * Build an unsupported platform exception with a simple message.
 * @param message	Exception message.
 */
UnsupportedPlatformException::UnsupportedPlatformException(const String& message)
: Exception(message) {
}

/**
 * Build an unsupported platform exception with a formatted message.
 * @param format	Format of the message.
 * @param args		Arguments of the format.
 */
UnsupportedPlatformException::UnsupportedPlatformException(const char *format, va_list args)
: Exception(format, args) {
}


/**
 * Build an unsupported platform exception with a formatted message.
 * @param format	Format of the message.
 * @param ...		Arguments of the format.
 */
UnsupportedPlatformException::UnsupportedPlatformException(const char *format, ...) {
	va_list args;
	va_start(args, format);
	build(format, args);
	va_end(args);
}


/**
 * @class Process
 * A process is the realization of a program on a platform. It represents the
 * program and its implementation on the platform. A process may be formed
 * by many files in case of shared object for example. A process provides the
 * information needed for simulating, analyzing or transforming a program.
 */

/**
 * @fn const Collection<File *> *Process::getFiles(void) const
 * Get the list of files used in this process.
 * @return	List of files.
 */

/**
 * @fn File *Process::createFile(void)
 * Build an empty file.
 * @return	The created file.
 */

/**
 * @fn	File *Process::loadFile(CString path)
 * Load an existing file.
 * @param path	Path to the file to load.
 * @return	The loaded file.
 * @exception LoadException							Error during the load.
 * @exception UnsupportedPlatformException	Platform of the file does
 * not match the platform of the process.
 */

/**
 * @fn Platform *Process::getPlatform(void) const
 * Get the platform of the process.
 * @return Process platform.
 */

/**
 * @fn Manager *Process::getManager(void) const
 * Get the manager owning this process.
 * @return Process manager.
 */


/**
 * @class Loader
 * This interface is implemented by all objects that may build and provide
 * a process. Many kind of loader may exists from the simple binary
 * loader to the complex program builder.
 */

/**
 * @fn Loader::~Loader(void)
 * Virtual destructor for destruction customization.
 */

/**
 * @fn CString Loader::getName(void) const
 * Get the name of the loader.
 * @return	Name of the loader.
 */

/**
 * @fn Process *Loader::load(Manager *man, CString path, PropList& props)
 * Load the file matching the given path with the given properties. The exact
 * type of the file and of the properties depends upon the underlying loader.
 * @param man		Caller manager.
 * @param path		Path to the file to load.
 * @param props	Property for loading.
 * @return Process containing the loaded file.
 * @exception LoadException							Error during the load.
 * @exception UnsupportedPlatformException	Loader does not handle
 * this platform.
 */

/**
 * @fn Process *Loader::create(Manager *man, PropList& props)
 * Build a new empty process matching the given properties.
 * @param man		Caller manager.
 * @param props	Properties describing the process.
 * @return Created process.
 * @exception UnsupportedPlatformException	Loader does not handle
 */

/**
 * Identifier of the property indicating the name (CString) of the platform to use.
 */	
id_t Loader::ID_PlatformName = Property::getID("otawa::PlatformName");

/**
 * Identifier of the property indicating a name (CString) of the loader to use..
 */	
id_t Loader::ID_LoaderName = Property::getID("otawa::LoaderName");

/**
 * Identifier of the property indicating a platform (Platform *) to use.
 */	
id_t Loader::ID_Platform = Property::getID("otawa::Platform");

/**
 * Identifier of the property indicating the loader to use.
 */	
id_t Loader::ID_Loader = Property::getID("otawa::Loader");

/**
 * Identifier of the property indicating the identifier (PlatformId) of the loader to use.
 */	
id_t Loader::ID_PlatformId = Property::getID("otawa::PlatformId");

/**
 * Name of the "Heptane" loader.
 */
CString Loader::LOADER_Heptane = "heptane";

/**
 * Name of the default PowerPC platform.
 */
CString Loader::PLATFORM_PowerPC_GLiss = "powerpc/ppc601-elf/gliss-gliss";


/**
 * @class FrameWork
 * A framework represents a program, its run-time and all information about
 * WCET computation or any other analysis.
 */

/**
 * Build a framework with the given process.
 * @param _proc	Process to use.
 */
FrameWork::FrameWork(Process *_proc): proc(_proc) {
	Manager *mgr = proc->manager();
	mgr->frameworks.add(this);
}

/**
 * Delete the framework and the associated process.
 */
FrameWork::~FrameWork(void) {
	Manager *mgr = proc->manager();
	mgr->frameworks.remove(this);
	delete proc;
}

/**
 * @fn Process *FrameWork::getProcess(void) const
 * Get the associated process.
 * @return	Associated process.
 */


/**
 * @class Manager
 * The manager class providesfacilities for storing, grouping and retrieving shared
 * resources like loaders and platforms.
 */

/**
 * Delete all used resources.
 */
Manager::~Manager(void) {
	for(int i = 0; i < frameworks.count(); i++)
		delete frameworks[i];
	for(int i = 0; i < loaders.count(); i++)
		delete loaders[i];
	for(int i = 0; i < platforms.count(); i++)
		delete platforms[i];
}


/**
 * Find the loader matching the given name.
 * @param name	Name of the loader to find.
 * @return				Found loader or null.
 * @note This function should also perform dynamic loading of load shared
 * library.
 */
Loader *Manager::findLoader(CString name) {
	for(int i = 0; i < loaders.count(); i++)
		if(loaders[i]->getName() == name)
			return loaders[i];
	return 0;
}

/**
 * Find a platform matching the given name.
 * @param name	Name of the platform to find.
 * @return				Found platform or null.
 */
Platform *Manager::findPlatform(CString name) {
	return findPlatform(PlatformId(name));
}

/**
 * Find a platform matching the given platform identifier.
 * @param id	Identifier of the platform.
 * @result			Found platform or null.
 */
Platform *Manager::findPlatform(const PlatformId& id) {
	for(int i = 0; i < platforms.count(); i++)
		if(platforms[i]->accept(id))
			return platforms[i];
	return 0;
}

/**
 * Load a file with the given path and the given properties.
 * @param path		Path of the file to load.
 * @param props	Properties describing the load process. It must contains
 * a loder property like ID_Loader or ID_LoaderName.
 * @return The loaded framework or 0.
 */
FrameWork *Manager::load(CString path, PropList& props) {
	
	// Get the loader
	Loader *loader = props.get<Loader *>(Loader::ID_Loader, 0);
	if(!loader) {
		CString name = props.get<CString>(Loader::ID_LoaderName, "");
		if(!name)
			return 0;
		loader = findLoader(name);
		if(!loader)
			return 0;
	}
	
	// Attempt to load the file
	Process *process =  loader->load(this, path, props);
	if(!process)
		return 0;
	else
		return new FrameWork(process);
}

/**
 * Manager builder. Install the PPC GLISS loader.
 */
Manager::Manager(void) {
	//loaders.add(&gliss::loader);
}

}

