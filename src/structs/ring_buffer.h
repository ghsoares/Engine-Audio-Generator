#ifndef STRUCTS_RING_BUFFER_H
#define STRUCTS_RING_BUFFER_H

template <typename T>
static inline T __rb_fposmod(T x, T y) {
	return x < 0 ? (y + x % y) : (x < y ? x : x % y);
}

template <typename T>
class RingBuffer {
    T *buffer;
    size_t size;

	inline void set(size_t pos, T val) {
		if (buffer == nullptr) return;
		this->buffer[__rb_fposmod(pos, this->size)] = val;
	}

	inline T get(size_t pos) const {
		if (buffer == nullptr) return T();
		return this->buffer[__rb_fposmod(pos, this->size)];
	}

	inline T* get_ptr(size_t pos) const {
		if (buffer == nullptr) return nullptr;
		return &this->buffer[__rb_fposmod(pos, this->size)];
	}

public:
	class Cursor {
		friend class RingBuffer;
		RingBuffer *buffer;

		size_t pos;

		inline Cursor(RingBuffer *buffer) {
			this->buffer = buffer;
			this->pos = 0;
		}
	public:
		inline void move(size_t of = 1) {
			this->pos = __rb_fposmod(this->pos + of, this->buffer->size);
		}

		inline void set_pos(size_t pos) {
			this->pos = __rb_fposmod(pos, this->buffer->size);
		}

		inline size_t get_pos() const {
			return this->pos;
		}

		inline void set(T val, size_t of = 0) {
			this->buffer->set(this->pos + of, val);
		}

		inline T get(size_t of = 0) const {
			return this->buffer->get(this->pos + of);
		}

		inline void add(T val, size_t of = 0) {
			T *bval = this->buffer->get_ptr(this->pos + of);
			if (bval != nullptr) *bval = *bval + val;
		}

		inline void reset() {
			this->pos = 0;
		}

		inline Cursor() {
			this->buffer = nullptr;
			this->pos = 0;
		}
	};

    inline void initialize(size_t size) {
		if (this->buffer != nullptr) {
			delete[] this->buffer;
			this->buffer = nullptr;
		}

        this->buffer = new T[size]();
        this->size = size;
    }

    inline void destroy() {
        if (this->buffer != nullptr) {
            delete[] this->buffer;
            this->buffer = nullptr;
        }

        this->size = 0;
    }

	inline void resize(size_t new_size) {
		if (new_size == this->size) return;

		if (this->buffer == nullptr) {
			this->initialize(new_size);
		} else {
			T *new_buffer = new T[new_size]();

			size_t prev_size = new_size < this->size ? new_size : this->size;

			for (size_t i = 0; i < prev_size; i++) {
				new_buffer[i] = this->buffer[i];
			}

			delete[] this->buffer;

			this->buffer = new_buffer;
			this->size = new_size;
		}
	}

	inline bool initialized() const {
		return this->buffer != nullptr;
	}

	inline size_t get_size() const {
		return this->size;
	}

	inline Cursor cursor() {
		return Cursor(this);
	}

	RingBuffer() {
        this->buffer = nullptr;
        this->size = 0;
    }

    ~RingBuffer() {
        destroy();
    }
};

#endif // STRUCTS_RING_BUFFER_H