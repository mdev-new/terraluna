#pragma once

#include <string>
#include <chrono>
#include <vector>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <string.h>
#include <mutex>

namespace Assets {

#define CHUNK_SIZE 4096

class CVFS;
class CVFSNode;
class CVFSFileStream;

using VFSNode = std::shared_ptr<CVFSNode>;
using VFSFileStream = std::shared_ptr<CVFSFileStream>;

enum class VFSError
{
	CANT_CREATE_DIR,
	CANT_CREATE_FILE,
	CANT_OPEN_FILE,
	OUT_OF_MEM,
	NODE_IS_FILE,
	NODE_IS_DIR,
	NODE_ALREADY_EXISTS,
	NODE_DOESNT_EXISTS,
	FAILED_TO_READ_STREAM,
	CANT_CREATE_FILESYSTEM
};

enum class FileMode
{
	READ = 1,
	WRITE = 2,
	RW = (READ | WRITE),
	APPEND = 4
};

inline FileMode operator | (FileMode lhs, FileMode rhs)
{
	return static_cast<FileMode>(static_cast<int>(lhs) | static_cast<int>(rhs));
}

inline FileMode& operator |= (FileMode& lhs, FileMode rhs)
{
	lhs = lhs | rhs;
	return lhs;
}

inline FileMode operator & (FileMode lhs, FileMode rhs)
{
	return static_cast<FileMode>(static_cast<int>(lhs) & static_cast<int>(rhs));
}

inline FileMode& operator &= (FileMode& lhs, FileMode rhs)
{
	lhs = lhs & rhs;
	return lhs;
}

enum class Cursor
{
	BEG,
	CUR,
	END
};

class CVFSException : public std::exception
{
	public:
		CVFSException() {}
		CVFSException(VFSError Type) : m_ErrType(Type) {}
		CVFSException(const std::string &Msg, VFSError Type) : m_Msg(Msg), m_ErrType(Type) {}

		const char *what() const noexcept override
		{
			return m_Msg.c_str();
		}

		VFSError GetErrType() const noexcept
		{
			return m_ErrType;
		}

	private:
		std::string m_Msg;
		VFSError m_ErrType;
};

class CVFSNode
{
	friend CVFS;

	public:
		CVFSNode()
		{
			m_Created = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			m_Accessed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
		}

		CVFSNode(const CVFSNode &node)
		{
			m_Name = node.m_Name;
			m_IsDir = node.m_IsDir;

			m_Created = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
			m_Accessed = node.m_Accessed;
		}

		inline std::string Name() const
		{
			std::lock_guard<std::mutex> lock(m_UpdateLock);
			return m_Name;
		}

		inline bool IsDir() const
		{
			std::lock_guard<std::mutex> lock(m_UpdateLock);
			return m_IsDir;
		}

		inline time_t Created() const
		{
			std::lock_guard<std::mutex> lock(m_UpdateLock);
			return m_Created;
		}
		
		inline time_t Accessed() const
		{
			std::lock_guard<std::mutex> lock(m_UpdateLock);
			return m_Accessed;
		}

		virtual VFSNode Copy() = 0;

		virtual ~CVFSNode() = default;

	protected:
		std::string m_Name;
		bool m_IsDir;

		time_t m_Created;
		time_t m_Accessed;

		mutable std::mutex m_UpdateLock;
};

class CVFS
{
	friend CVFSFileStream;

	public:
		CVFS(/* args */) 
		{
			m_Root = VFSDir(new CVFSDir("/"));
		}

		void CreateDir(const std::string &Path, bool Force = false)
		{
			auto Dirs = SplitPath(Path);
			auto CurDir = m_Root;

			for (size_t i = 0; i < Dirs.size(); i++)
			{
				std::string Dir = Dirs[i];

				auto node = CurDir->Search(Dir);

				if(!node && (Force || (i == Dirs.size() - 1)))
				{
					VFSDir tmp;
					try
					{
						tmp = VFSDir(new CVFSDir(Dir));
						CurDir->AppendChild(tmp);
					}
					catch(const std::bad_alloc &e)
					{
						throw CVFSException("Can't create directory. Out of mem. bad_alloc: " + std::string(e.what()), VFSError::OUT_OF_MEM);
					}
					
					CurDir = tmp;
				}
				else if(!node || !node->IsDir())
					throw CVFSException("Can't create directory", VFSError::CANT_CREATE_DIR);
				else
					CurDir = std::static_pointer_cast<CVFSDir>(node);
			}               
		}

		VFSNode GetNodeInfo(const std::string &Path)
		{
			auto Dirs = SplitPath(Path);
			auto CurDir = m_Root;
			VFSNode Ret;
			if(Path == "/")
				Ret = m_Root;

			for (size_t i = 0; i < Dirs.size(); i++)
			{
				std::string Dir = Dirs[i];

				auto node = CurDir->Search(Dir);
				if(node && (node->IsDir() || (i == Dirs.size() - 1)))   //"Go into" the directory. 
				{
					Ret = node;
					CurDir = std::static_pointer_cast<CVFSDir>(node);
				}
				else
				{
					Ret = nullptr;
					break;
				}
			}

			return Ret;
		}

		bool NodeExists(const std::string &Path)
		{
			return GetNodeInfo(Path) != nullptr; 
		}

		std::vector<VFSNode> List(const std::string &Path)
		{
			auto node = GetNodeInfo(Path);
			return List(node);
		}

		std::vector<VFSNode> List(VFSNode node)
		{
			std::vector<VFSNode> Ret;
			if(node && node->IsDir())
			{
				auto Dir = std::static_pointer_cast<CVFSDir>(node);
				Ret = Dir->GetChilds();
			}
			else if(node && !node->IsDir())
				throw CVFSException("Given node is not a directory", VFSError::NODE_IS_FILE);

			return Ret;
		}

		VFSFileStream Open(const std::string &Path, FileMode mode);

		size_t FileSize(VFSNode node)
		{
			if(!node->IsDir())
			{
				auto file = std::static_pointer_cast<CVFSFile>(node);
				return file->Size();
			}
			else if(node && !node->IsDir())
				throw CVFSException("Given node is not a file.", VFSError::NODE_IS_DIR);

			return 0;
		}

		void Rename(const std::string &Path, const std::string &Name)
		{
			if(NodeExists(Path))
			{
				if(!NodeExists(ExtractPath(Path) + "/" + Name))
				{
					auto Parent = std::static_pointer_cast<CVFSDir>(GetNodeInfo(ExtractPath(Path)));
					Parent->RenameChild(ExtractName(Path), Name);
				}
				else
					throw CVFSException("Can't rename node. Node already exists.", VFSError::NODE_ALREADY_EXISTS);
			}
			else
				throw CVFSException("Can't rename node. Node doesn't exists.", VFSError::NODE_DOESNT_EXISTS);
		}

		void Move(const std::string &From, const std::string &To)
		{
			if(!NodeExists(From))
				throw CVFSException("Can't move node. Source node doesn't exists.", VFSError::NODE_DOESNT_EXISTS);

			if(!NodeExists(To))
				throw CVFSException("Can't move node. Destination node doesn't exists.", VFSError::NODE_DOESNT_EXISTS);

			auto DestNode = GetNodeInfo(To);
			if(!DestNode->IsDir())
				throw CVFSException("Can't move node. Destination node is a file.", VFSError::NODE_IS_FILE);

			auto node = GetNodeInfo(From);
			auto SrcParent = std::static_pointer_cast<CVFSDir>(GetNodeInfo(ExtractPath(From)));
			auto DestParent = std::static_pointer_cast<CVFSDir>(DestNode);

			SrcParent->RemoveChild(node->Name());
			DestParent->AppendChild(node);
		}

		void Delete(const std::string &Path)
		{
			if(!NodeExists(Path))
				throw CVFSException("Can't delete node. Node doesn't exists.", VFSError::NODE_DOESNT_EXISTS);

			auto node = GetNodeInfo(Path);
			auto Parent = std::static_pointer_cast<CVFSDir>(GetNodeInfo(ExtractPath(Path)));

			Parent->RemoveChild(node->Name());
		}

		void Copy(const std::string &From, const std::string &To)
		{
			if(!NodeExists(From))
				throw CVFSException("Can't copy node. Source node doesn't exists.", VFSError::NODE_DOESNT_EXISTS);

			if(NodeExists(To))
				throw CVFSException("Can't copy node. Destination node already exists.", VFSError::NODE_ALREADY_EXISTS);

			auto DestNode = GetNodeInfo(ExtractPath(To));
			if(!DestNode->IsDir())
				throw CVFSException("Can't copy node. Destination node parent is a file.", VFSError::NODE_IS_FILE);

			auto node = GetNodeInfo(From);
			auto DestParent = std::static_pointer_cast<CVFSDir>(DestNode);

			auto copy = node->Copy();
			copy->m_Name = ExtractName(To);

			DestParent->AppendChild(copy);
		}

		std::vector<char> Serialize()
		{
			try
			{
				VFSFile Disk = VFSFile(new CVFSFile("stream"));
				Disk->Clear();
				Disk->Write(MAGIC.data(), MAGIC.size());

				uint64_t Entries = m_Root->GetChilds().size();
				Disk->Write((char*)&Entries, sizeof(Entries));
				FillSpace(Disk.get(), DISK_CHUNK_SIZE - (MAGIC.size() + sizeof(Entries)));

				auto Childs = m_Root->GetChilds();
				for (auto e : Childs)
					SerializeNode(Disk.get(), e.get());

				std::vector<char> Ret;
				Ret.resize(Disk->Size());
				Disk->Read(&Ret[0], Disk->Size(), 0);
				return Ret;
			}
			catch(const std::bad_alloc &e)
			{
				throw CVFSException("Can't create stream. Out of mem. bad_alloc: " + std::string(e.what()), VFSError::OUT_OF_MEM);
			}
		}

		void Deserialize(const std::vector<char> &Data)
		{
			try
			{
				size_t Pos = 0;
				uint64_t Entries = 0;
				std::string FileMagic;
				FileMagic.resize(MAGIC.size());

				ReadVector(Data, &FileMagic[0], FileMagic.size(), Pos);
				if(FileMagic != MAGIC)
					throw CVFSException("Can't create filesystem.", VFSError::CANT_CREATE_FILESYSTEM);

				ReadVector(Data, (char*)&Entries, sizeof(Entries), Pos);

				Pos += (DISK_CHUNK_SIZE - (MAGIC.size() + sizeof(Entries)));

				for (size_t i = 0; i < Entries; i++)
					m_Root->AppendChild(DeserializeNode(Data, Pos));
			
			}
			catch(const std::bad_alloc &e)
			{
				throw CVFSException("Can't create filesystem. Out of mem. bad_alloc: " + std::string(e.what()), VFSError::OUT_OF_MEM);
			}
		}

		size_t ReadVector(const std::vector<char> &Data, char *Buf, size_t Size, size_t &Pos)
		{
			size_t CopyCount = (Pos + Size) < Data.size() ? Size : (Data.size() - Pos);
			if(CopyCount > Data.size())
			   throw CVFSException("Can't create filesystem.", VFSError::FAILED_TO_READ_STREAM);

			memcpy(Buf, Data.data() + Pos, CopyCount);
			Pos += CopyCount;

			return CopyCount;
		}

		~CVFS() {}
	private:
		const std::string MAGIC = "CVFS-DISK";
		const int DISK_CHUNK_SIZE = 128;
		const std::string NODE_IDENTIFIER = "NODE";

		class CVFSFile;
		class CVFSDir;

		using VFSFile = std::shared_ptr<CVFSFile>;
		using VFSDir = std::shared_ptr<CVFSDir>;
		
		class CVFSFile : public CVFSNode
		{
			friend CVFS;

			public:
				CVFSFile() : CVFSNode()
				{
					m_IsDir = false;
					m_Modified = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					m_Size = 0;
				}

				CVFSFile(const std::string &Name) : CVFSFile()
				{
					m_Name = Name;
				}

				CVFSFile(const CVFSFile &file) : CVFSNode(file)
				{
					m_Modified = file.m_Modified;
					m_Size = file.m_Size;

					ReserveChunks(file.m_Data.size());

					for (size_t i = 0; i < file.m_Data.size(); i++)
					{
						Chunk s = file.m_Data[i];
						Chunk d = m_Data[i];

						d->Filled = s->Filled;
						memcpy(d->Data, s->Data, s->Filled);
					}
				}

				void Clear()
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					m_Data.clear();
					ReserveChunks(4);
				}

				size_t Write(const char *Data, size_t Size)
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);

					size_t ChunkCount = Size / CHUNK_SIZE + ((Size % CHUNK_SIZE > 0) ? 1 : 0);
					if(((m_Size + Size) >= (m_Data.size() * CHUNK_SIZE)))    //Allocate new chunks, if we are exhausted.
						ReserveChunks(ChunkCount);

					size_t Written = 0;
					size_t ChunkPos = m_Size / CHUNK_SIZE;  //Calculates the beginning chunk.

					while (Written < Size)
					{
						Chunk c = m_Data[ChunkPos];
						size_t Free = c->Size - c->Filled;
						size_t CopyCount = ((Size - Written) >= Free) ? Free : (Size - Written);    //Calculate the right copy size.

						memcpy(c->Data + c->Filled, Data + Written, CopyCount);
						c->Filled += CopyCount; //Chunk update
						m_Size += CopyCount;  
						Written += CopyCount;
						ChunkPos++;
					}

					m_Modified = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					return Written;
				}

				size_t Read(char *Buf, size_t Size, size_t CurPos)
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);

					size_t ChunkPos = CurPos / CHUNK_SIZE; //Calculates the beginning chunk.
					size_t Readed = 0;

					while (Readed < Size)
					{
						if(ChunkPos >= m_Data.size())
							break;

						Chunk c = m_Data[ChunkPos];
						size_t Pos = CurPos - ChunkPos * CHUNK_SIZE;
						Pos = (Pos > CHUNK_SIZE) ? 0 : Pos;

						int CopyCount = c->Filled - Pos;
						CopyCount = CopyCount > Size ? Size : CopyCount;    //Calculate the right copy size.
						if(CopyCount <= 0)
							break;

						memcpy(Buf + Readed, c->Data + Pos, CopyCount);
						Readed += CopyCount;
						ChunkPos++;
					}
					
					m_Accessed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					return Readed;
				}

				inline time_t Modified() const
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					return m_Modified;
				}


				inline size_t Size() const
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					return m_Size;
				}

				VFSNode Copy() override
				{
					return VFSFile(new CVFSFile(*this));
				}

			private:
				struct SChunk
				{
					public:
						SChunk()
						{
							Size = CHUNK_SIZE;
							Filled = 0;
							Data = new char[Size];
						}

						int Size;
						int Filled;
						char *Data;

						~SChunk()
						{
							delete[] Data;
						}
				};

				using Chunk = std::shared_ptr<SChunk>;

				void ReserveChunks(size_t Count)
				{
					m_Data.reserve(Count);
					for (size_t i = 0; i < Count; i++)
						m_Data.push_back(Chunk(new SChunk()));
				}

				time_t m_Modified;
				size_t m_Size;

				std::vector<Chunk> m_Data;
		};

		class CVFSDir : public CVFSNode
		{
			public:
				CVFSDir() : CVFSNode()
				{
					m_IsDir = true;
				}

				CVFSDir(const std::string &Name) : CVFSDir()
				{
					m_Name = Name;
				}

				CVFSDir(const CVFSDir &dir) : CVFSNode(dir)
				{
					m_Childs.reserve(dir.m_Childs.size());
					for (auto &&e : dir.m_Childs)
					{
						m_Childs.push_back(e->Copy());
					}
				}

				void AppendChild(VFSNode Child)
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					InternalAppendChild(Child);
				}

				VFSNode Search(const std::string &Name)
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					VFSNode Ret;

					if(!m_Childs.empty())
					{
						size_t Pos = Search(Name, 0, m_Childs.size() - 1);
						if(m_Childs[Pos]->Name() == Name)
							Ret = m_Childs[Pos];
					}

					return Ret;
				}

				void RenameChild(const std::string &Name, const std::string &NewName)
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					size_t Pos = Search(Name, 0, m_Childs.size() - 1);
					auto Child = m_Childs[Pos];
					if(Child->Name() == Name)
					{
						m_Childs.erase(m_Childs.begin() + Pos); //Removes the child temporary.
						Child->m_Name = NewName;

						InternalAppendChild(Child);
					}
				}

				void RemoveChild(const std::string &Name)
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					size_t Pos = Search(Name, 0, m_Childs.size() - 1);
					auto Child = m_Childs[Pos];
					if(Child->Name() == Name)
						m_Childs.erase(m_Childs.begin() + Pos); //Removes the child.
				}

				std::vector<VFSNode> GetChilds()
				{
					std::lock_guard<std::mutex> lock(m_UpdateLock);
					m_Accessed = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
					return m_Childs;
				}

				VFSNode Copy() override
				{
					return VFSDir(new CVFSDir(*this));
				}

			private:
				size_t Search(const std::string &Name, size_t Start, size_t End)
				{
					if(End < Start || End == (size_t) -1)
						return 0;
					else if(End == Start)
						return End;

					size_t Middle = Start + (End - Start) / 2;
					std::string PosName = m_Childs[Middle]->Name();
					if(PosName == Name)
						return Middle;
					else if(Name > PosName)
						return Search(Name, Middle + 1, End);
					else if(Name < PosName)
						return Search(Name, Start, Middle - 1);

					return Middle;                        
				}

				void InternalAppendChild(VFSNode Child)
				{
					size_t Pos = 0;

					for (Pos = 0; Pos < m_Childs.size(); Pos++)
					{
						if(Child->Name() < m_Childs[Pos]->Name())
							break;
					}
					
					m_Childs.insert(m_Childs.begin() + Pos, Child);
				}

				std::vector<VFSNode> m_Childs;
		};

		std::vector<std::string> SplitPath(const std::string &Path)
		{
			size_t Pos = 0;
			std::vector<std::string> Ret;

			if(!Path.empty() && Path[0] == '/')
				Pos++;

			while (Pos != std::string::npos)
			{
				size_t Start = Pos;
				Pos = Path.find('/', Start);

				std::string Dir = Path.substr(Start, Pos - Start);
				
				if(!Dir.empty())
					Ret.push_back(Dir);

				if(Pos != std::string::npos)
					Pos++;
			}
			
			return Ret;
		}

		std::string ExtractPath(const std::string &Path)
		{
			size_t Pos = Path.find_last_of("/");
			if(Pos == Path.length() - 1)
				Pos = Path.find_last_of("/", Pos - 1);

			return Path.substr(0, Pos);
		}

		std::string ExtractName(const std::string &Path)
		{
			size_t End = std::string::npos;
			size_t Pos = Path.find_last_of("/");
			if(Pos == Path.length() - 1)
			{
				End = Pos - 1;
				Pos = Path.find_last_of("/", Pos - 1);
			}

			return Path.substr(Pos + 1, End - Pos);
		}

		void FillSpace(CVFSFile *file, size_t Count)
		{
			for (size_t i = 0; i < Count; i++)
			{
				char c = 0;
				file->Write(&c, sizeof(c));
			}
		}
		
		void SerializeNode(CVFSFile *file, CVFSNode *Node)
		{
			size_t NodeSize = NODE_IDENTIFIER.size() + sizeof(int) + (int)Node->m_Name.size() + sizeof(Node->m_IsDir) + sizeof(Node->m_Created) + sizeof(Node->m_Accessed);
			file->Write(NODE_IDENTIFIER.data(), NODE_IDENTIFIER.size());

			int NameSize = Node->m_Name.size();
			file->Write((char*)&NameSize, sizeof(int));
			file->Write(Node->m_Name.data(), NameSize);
			file->Write((char*)&Node->m_IsDir, sizeof(Node->m_IsDir));
			file->Write((char*)&Node->m_Created, sizeof(Node->m_Created));
			file->Write((char*)&Node->m_Accessed, sizeof(Node->m_Accessed));

			if(Node->IsDir())
			{
				auto Dir = static_cast<CVFSDir*>(Node);
				auto Childs = Dir->GetChilds();

				uint64_t EntryCount = Childs.size();
				file->Write((char*)&EntryCount, sizeof(EntryCount));

				int FillSize = DISK_CHUNK_SIZE - (NodeSize + sizeof(uint64_t));
				FillSpace(file, FillSize);
				for (auto e : Childs)
					SerializeNode(file, e.get());
			}
			else
			{
				auto NodeFile = static_cast<CVFSFile*>(Node);

				time_t mtime = NodeFile->Modified();
				file->Write((char*)&mtime, sizeof(mtime));

				uint64_t Size = NodeFile->Size();
				file->Write((char*)&Size, sizeof(Size));

				NodeSize += sizeof(mtime) + sizeof(Size);

				int FillSize = DISK_CHUNK_SIZE - (NodeSize > DISK_CHUNK_SIZE ? (NodeSize - DISK_CHUNK_SIZE) : NodeSize);
				if(NodeFile->Size() <= FillSize)
				{
					file->Write(NodeFile->m_Data[0]->Data, NodeFile->m_Data[0]->Filled);
					FillSize -= NodeFile->Size();
				}

				FillSpace(file, FillSize);
				if(NodeFile->Size() > FillSize)
				{
					FillSize = DISK_CHUNK_SIZE - (NodeFile->Size() % DISK_CHUNK_SIZE);

					for (auto e : NodeFile->m_Data)
					{
						if(e->Filled == 0)
							break;

						file->Write(e->Data, e->Filled);
					}

					if(FillSize < DISK_CHUNK_SIZE)
						FillSpace(file, FillSize);
				}
			}
		}

		VFSNode DeserializeNode(const std::vector<char> &Data, size_t &Pos)
		{
			std::string Identifier(NODE_IDENTIFIER.size(), '\0');
			ReadVector(Data, &Identifier[0], Identifier.size(), Pos);
			if(Identifier != NODE_IDENTIFIER)
				throw CVFSException("Invalied node identifier!", VFSError::CANT_CREATE_FILESYSTEM);

			int NameSize = 0;
			ReadVector(Data, (char*)&NameSize, sizeof(NameSize), Pos); 

			std::string Name(NameSize, '\0');
			ReadVector(Data, &Name[0], Name.size(), Pos);

			bool IsDir;
			ReadVector(Data, (char*)&IsDir, sizeof(IsDir), Pos); 

			time_t Created;
			time_t Accessed;
			ReadVector(Data, (char*)&Created, sizeof(Created), Pos); 
			ReadVector(Data, (char*)&Accessed, sizeof(Accessed), Pos); 

			if(IsDir)
			{
				auto Dir = VFSDir(new CVFSDir(Name));
				Dir->m_Created = Created;
				Dir->m_Accessed = Accessed;

				uint64_t Entries = 0;
				ReadVector(Data, (char*)&Entries, sizeof(Entries), Pos); 

				size_t NodeSize = NODE_IDENTIFIER.size() + sizeof(int) + (int)Dir->m_Name.size() + sizeof(Dir->m_IsDir) + sizeof(Dir->m_Created) + sizeof(Dir->m_Accessed) + sizeof(uint64_t);

				Pos += DISK_CHUNK_SIZE - NodeSize;

				for (size_t i = 0; i < Entries; i++)
					Dir->AppendChild(DeserializeNode(Data, Pos));
		
				return Dir;
			}
			else
			{
				auto File = VFSFile(new CVFSFile(Name));

				time_t mtime;
				ReadVector(Data, (char*)&mtime, sizeof(mtime), Pos); 

				File->m_Created = Created;
				File->m_Accessed = Accessed;
				File->m_Modified = mtime;

				uint64_t Size;
				ReadVector(Data, (char*)&Size, sizeof(Size), Pos);

				size_t NodeSize = NODE_IDENTIFIER.size() + sizeof(int) + (int)File->m_Name.size() + sizeof(File->m_IsDir) + sizeof(File->m_Created) + sizeof(File->m_Accessed) + sizeof(mtime) + sizeof(Size);
				int FillSize = DISK_CHUNK_SIZE - (NodeSize > DISK_CHUNK_SIZE ? (NodeSize - DISK_CHUNK_SIZE) : NodeSize);

				if(Size > FillSize)
					Pos += FillSize;

				std::string Buf(Size, '\0');
				ReadVector(Data, &Buf[0], Buf.size(), Pos);
				File->Write(Buf.data(), Buf.size());

				if(Size <= FillSize)
					Pos += FillSize - Size;
				else
					Pos += DISK_CHUNK_SIZE - (Size % DISK_CHUNK_SIZE);

				return File;
			}
		}

		VFSDir m_Root;
};

class CVFSFileStream
{
	public:
		CVFSFileStream(CVFS::VFSFile file, FileMode mode) : m_File(file), m_Mode(mode), m_CurPos(0)
		{
			if((mode & FileMode::APPEND) != FileMode::APPEND)
				m_File->Clear();
		}

		size_t WriteLine(const std::string &Line)
		{
			char c = '\n';
			size_t Ret = Write(Line.data(), Line.size());
			Ret += Write(&c, sizeof(c));

			return Ret;
		}

		size_t Write(const std::string &Str)
		{
			return Write(Str.data(), Str.size());
		}

		inline size_t Write(const char *Data, size_t Size)
		{
			if((m_Mode & FileMode::WRITE) == FileMode::WRITE)
			{
				return m_File->Write(Data, Size);
			}

			return 0;
		}

		std::string ReadLine()
		{
			std::string Ret;
			char c;
			while (Read(&c, sizeof(c)) != 0)
			{
				if(c == '\n')
					break;

				Ret += c;
			}

			return Ret;
		}

		std::string Read()
		{
			std::string Ret;
			Ret.resize(Size());
			Read(&Ret[0], Ret.size());

			return Ret;
		}

		inline size_t Read(char *Buf, size_t Size)
		{
			if(((m_Mode & FileMode::READ) == FileMode::READ) && this->Size() != 0)
			{
				size_t Ret = m_File->Read(Buf, Size, m_CurPos);
				m_CurPos += Ret;
				return Ret;
			}

			return 0;
		}

		inline void Seek(Cursor cur, int64_t Bytes)
		{
			if(Size() == 0)
				return;

			switch (cur)
			{
				case Cursor::BEG:
				{
					m_CurPos = (size_t)Bytes > Size() ? Size() : Bytes;
				}break;

				case Cursor::CUR:
				{
					size_t NewPos = m_CurPos + Bytes;
					m_CurPos = NewPos > Size() ? Size() : NewPos;
				}break;

				case Cursor::END:
				{
					size_t NewPos = Size() + Bytes;
					m_CurPos = NewPos > Size() ? Size() : NewPos;
				}break;
			}
		}

		inline size_t Tell() const
		{
			return m_CurPos;
		}

		inline size_t Size() const
		{
			return m_File->Size();
		}

		inline bool IsEOF() const
		{
			return m_CurPos >= Size();
		}

		inline std::string Name() const
		{
			return m_File->Name();
		}

		virtual ~CVFSFileStream() {}
	private:
		CVFS::VFSFile m_File;
		FileMode m_Mode;

		size_t m_CurPos;
};

inline VFSFileStream CVFS::Open(const std::string &Path, FileMode mode)
{
	VFSFileStream ret;
	auto node = GetNodeInfo(Path);
	if(node && !node->IsDir())
		ret = VFSFileStream(new CVFSFileStream(std::static_pointer_cast<CVFSFile>(node), mode));
	else if(node && node->IsDir())
		throw CVFSException("Can't open file. A directory with the given name already exists.", VFSError::CANT_CREATE_FILE);
	else if((mode & FileMode::WRITE) == FileMode::WRITE)    //Creates a new file.
	{
		node = GetNodeInfo(ExtractPath(Path));
		if(node)
		{
			auto file = VFSFile(new CVFSFile(ExtractName(Path)));
			auto dir = std::static_pointer_cast<CVFSDir>(node);
			dir->AppendChild(file);
			ret = VFSFileStream(new CVFSFileStream(file, mode));
		}
	}
	else
		throw CVFSException("Can't open file. File doesn't exists.", VFSError::CANT_OPEN_FILE);

	return ret;
}
} // namespace Assets