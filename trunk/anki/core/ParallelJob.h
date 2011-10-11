#ifndef ANKI_CORE_PARALLEL_JOB_H
#define ANKI_CORE_PARALLEL_JOB_H

#include <boost/thread.hpp>


namespace anki {


class ParallelManager;
class ParallelJob;


/// The base class of the parameters the we pass in the job
struct ParallelJobParameters
{};


/// The callback that we feed to the job
typedef void (*ParallelJobCallback)(ParallelJobParameters&, const ParallelJob&);


/// The thread that executes a ParallelJobCallback
class ParallelJob
{
	public:
		/// Constructor
		ParallelJob(int id, const ParallelManager& manager,
			boost::barrier& barrier);

		/// @name Accessors
		/// @{
		uint getId() const
		{
			return id;
		}

		const ParallelManager& getManager() const
		{
			return manager;
		}
		/// @}

		/// Assign new job to the thread
		void assignNewJob(ParallelJobCallback callback, ParallelJobParameters& jobParams);

	private:
		uint id; ///< An ID
		boost::thread thread; ///< Runs the workingFunc
		boost::mutex mutex; ///< Protect the ParallelJob::job
		boost::condition_variable condVar; ///< To wake up the thread
		boost::barrier& barrier; ///< For synchronization
		ParallelJobCallback callback; ///< Its NULL if there are no pending job
		ParallelJobParameters* params;
		/// Know your father and pass him to the jobs
		const ParallelManager& manager;

		/// Start thread
		void start()
		{
			thread = boost::thread(&ParallelJob::workingFunc, this);
		}

		/// Thread loop
		void workingFunc();
};


} // end namespace


#endif
