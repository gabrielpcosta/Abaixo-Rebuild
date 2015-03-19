namespace abx {
	
	struct SharedPtrHolder {
		virtual ~SharedPtrHolder() {
			
		}
	};
	
	template <class T>
	class SharedPtrWrap : public SharedPtrHolder {
		public:
			template <typename... TArgs>
			SharedPtrWrap (TArgs&&... args)
				: m_ptr(std::forward<TArgs>(args)...)
			{	}
		
		private:
			std::shared_ptr<T> m_ptr;
	};
	
	class ResourceOwner {
		private:
			std::vector<std::unique_ptr<SharedPtrWrap>> m_resources;
			
			template <typename>
			friend ResourceHolder;
			
		public:
			void ownResource (ResourceHolderBase& resHolder) {
				resHolder.makeOwner(*this);
			}
			
			void disownAllResources() {
				resHolder.clear();
			}
	};
	
}

namespace abx {
	
	class ResourceHolderBase {
		private:
			virtual void makeOwner (ResourceOwner&) =0;
			
			friend ResourceOwner;
	};
	
	template <class R, class S = DefaultStream>
	class ResourceHolder : public ResourceHolderBase, public sf::NonCopyable {
		public:
			typedef std::shared_ptr<R> Pointer;
			typedef T Type;
			typedef S Stream;
			
			void unloadIfUnused() {
				if (m_res.unique())
					m_res = nullptr;
			}
			
			const Type& getResource() {
				if (!m_res)
					onLoadResource();
				
				return *m_res;
			}
			
			~ResourceHolder() {
				m_stream.close();
			}
		
		private:
			 Pointer m_res;
			 Stream m_stream;
			 
			virtual void setToken (std::string&& token) =0;
			virtual void onLoadResource() =0;
			
			void makeOwner (ResourceOwner& owner) override {
				owner.m_resources.emplace(new SharedPtrWrap<R>(m_res));
			}
			
			template <typename>
			friend class ResourceManager;
		
		protected:
			Pointer& getResourcePointer() {
				return m_res;
			}
			
			const Pointer& getResourcePointer() const {
				return m_res;
			}
			
			Stream& getStream();
			const Stream& getStream() const;
	};
	
}

namespace abx {
	
	class AnimationResource : public ResourceHolder<AnimationGroup> {
		private:
			std::string m_folderToken;
			
			void setToken (std::string&& token) override {
				m_folderToken = std::move(token);
				getStream().open(AnimationGroup::makeFilePath(token));
			}
			
			void onLoadResource() override {
				auto res& = getResourcePointer();
				res = std::make_shared<AnimationGroup>();
				
				res->setFolderName(m_folderToken);
				res->loadFromStream(getStream());
			}
	};
	
}

namespace abx {
	
	template <typename K, class RPointer>
	class ResourceManager : public sf::NonCopyable {
		public:
			typedef K Key;
			typedef RPointer ResourcePtr;
			typedef ResourcePtr::Type Resource;
		
			void registerResource (Key key, std::string&& token, bool loadASAP = false) {
				auto result = m_map.emplace(
						std::piecewise_construct,
						std::forward_as_tuple(std::move(key)),
						std::forward_as_tuple()
					);
				
				// Se result.second == false, então já havia uma resource registrada sob esta chave e o novo
				// registro não foi realizado.
				ABX_ASSERT(result.second);
				
				auto& resource = makeFriendly(result.iter->second);
				resource->setToken(std::move(token));
				
				// Cheque se é preferível que a resource seja carregada de imediato (ao invés que seja sob demanda);
				if (loadASAP) {
					resource->onLoadResource();
				}
			};
			
			Resource& retrieveResource (const Key& key); {
				return m_map.at(key).getResource();
			}
			
			void unloadAllUnused() {
				for (auto& pair: m_map) {
					auto& resPtr = pair.second;
					resPtr.unloadIfUnused();
				}
			}
		
		private:
			std::map<Id, ResourcePtr> m_map;
			
			/// Função auxiliar que converte a ResourcePtr do ResourceManager em um abx::ResourceHolder<>, para que
			/// possamos acessar os métodos ::onLoadResource e ::setToken, métodos privados para o qual um
			/// abx::ResourceManager possui acesso por ser amigo.
			///
			/// Ela é necessária pois caso ResourcePtr for uma classe derivada de ResourceHolder (quando não é um
			/// ResourceHolder de cara) então ResourceManager não terá acesso aos métodos privados de sua classe
			/// base mesmo com a amizade.
			///
			template <typename... TArgs>
			static ResourceHolder<TArgs...>& makeFriendly (ResourcePtr& resPtr) {
				return resPtr;
			}
	};
	
}

namespace abx {
	
	enum class Animations;
	
	class MasterManager {
		public:
			static void registerGlobalResources();
			
			void registerAnimation (Animations key, std::string&& folder, bool loadASAP = false) {
				m_anims.registerResource(key, std::move(folder), loadASAP);
			}
			
		private:
			ResourceManager<Animations, AnimationResource> m_anims;
	};
	
}

namespace abx {
	
	enum class Animations {
		Pix,
		Chip,
		Heevahava
	};
	
}

namespace abx {
	
	void MasterManager::registerGlobalResources() {
		
		#define _(KEY, FOLDER) our_mgr.registerAnimation(KEY, FOLDER, true)
		
		_(Animations::Pix, "anim-pix"); // data/Animations/anim-pix/.anim
		_(Animations::Chip, "anim-chip");
		_(Animations::Heevahava, "anim-heeva");
		
	}
	
}