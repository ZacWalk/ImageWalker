#pragma once


namespace IW
{

	class ReadWriteLock
	{
		//  Interface

	public:
		//  Constructor

		ReadWriteLock()
		{
			m_ReaderCount = 0;
			m_WriterCount = 0;
		}

		//  Destructor

		~ReadWriteLock()
		{
			_ASSERTE( m_ReaderCount == 0 );
			_ASSERTE( m_WriterCount == 0 );
		}

		//  Reader lock acquisition and release

		void LockForReading()
		{
			while( 1 )
			{
				//  If there's a writer already, spin without unnecessarily
				//  interlocking the CPUs

				if( m_WriterCount != 0 )
				{
					Sleep(0);
					continue;
				}

				//  Add to the readers list

				InterlockedIncrement((long*) &m_ReaderCount );

				//  Check for writers again (we may have been pre-empted). If
				//  there are no writers writing or waiting, then we're done.

				if( m_WriterCount == 0 )
					break;

				//  Remove from the readers list, spin, try again

				InterlockedDecrement((long*) &m_ReaderCount );
				Sleep(0);
			}
		}

		void UnlockForReading()
		{
			InterlockedDecrement((long*) &m_ReaderCount );
		}

		//  Writer lock acquisition and release

		void LockForWriting()
		{
			//  See if we can become the writer (expensive, because it inter-
			//  locks the CPUs, so writing should be an infrequent process)

			while( InterlockedExchange((long*) &m_WriterCount, 1 ) == 1 )
			{
				Sleep(0);
			}

			//  Now we're the writer, but there may be outstanding readers.
			//  Spin until there aren't any more; new readers will wait now
			//  that we're the writer.

			while( m_ReaderCount != 0 )
			{
				Sleep(0);
			}
		}

		void UnlockForWriting()
		{
			m_WriterCount = 0;
		}

		//  Implementation

	private:
		long volatile m_ReaderCount;
		long volatile m_WriterCount;
	};

	class ReadLock
	{
	public:
		ReadWriteLock &_lock;

		ReadLock(ReadWriteLock &lock) : _lock(lock)
		{
			_lock.LockForReading();
		}

		~ReadLock()
		{
			_lock.UnlockForReading();
		}
	};

	class WriteLock
	{
	public:
		ReadWriteLock &_lock;

		WriteLock(ReadWriteLock &lock) : _lock(lock)
		{
			_lock.LockForWriting();
		}

		~WriteLock()
		{
			_lock.UnlockForWriting();
		}
	};

}   //  End of namespace
