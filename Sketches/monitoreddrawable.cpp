namespace abx {
	
	class MonitoredDrawable : public sf::NonCopyable {
		public:
			 MonitoredDrawable();
			~MonitoredDrawable();
			
			void setZLayer (float);
			float getZLayer() const;
			
			void setLayerGroup (LayerGroup&);
			const LayerGroup* getLayerGroup() const;
			
		protected:
			void setTextureId (size_t);
			size_t getTextureId() const;
			
		private:
			float m_zLayer;
			
			size_t m_texId;
			
			LayerGroup *m_group;
			
			void removeFromGroup();
			void updateIntoGroup();
	};
	
}

namespace abx {
	
	MonitoredDrawable::MonitoredDrawable()
		: m_zLayer(0)
		, m_texId(0)
		, m_group(nullptr)
	{	}
	
	MonitoredDrawable::~MonitoredDrawable() {
		removeFromGroup();
	}
	
	// ---
	
	void MonitoredDrawable::setZLayer (float z) {
		if (z != m_zLayer) {
			removeFromGroup();
			m_zLayer = z;
			updateIntoGroup();
		}
	}
	
	float MonitoredDrawable::getZLayer() const {
		return m_zLayer;
	}
	
	// ---
	
	void MonitoredDrawable::setLayerGroup (LayerGroup& group) {
		if (&group != m_group) {
			removeFromGroup();
			m_group = &group;
			updateIntoGroup();
		}
	}
	
	const LayerGroup* MonitoredDrawable::getLayerGroup() const {
		return m_group;
	}
	
	// ---
	
	void MonitoredDrawable::setTextureId (size_t id) {
		if (m_texId != id) {
			removeFromGroup();
			m_texId = id;
			updateIntoGroup();
		}
	}
	
	size_t MonitoredDrawable::getTextureId() const {
		return m_texId;
	}
	
	// ---
	
	void MonitoredDrawable::removeFromGroup() {
		m_group->eraseDrawable(m_zLayer, m_texId, *this);
	}
	
	void MonitoredDrawable::updateIntoGroup() {
		m_group->insertDrawable(m_zLayer, m_texId, *this);
	}
	
}