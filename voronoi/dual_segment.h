
/**
* author: Yanyang Xiao
* email : yanyangxiaoxyy@gmail.com
*/

#ifndef DUAL_SEGMENT_H
#define DUAL_SEGMENT_H

namespace xyy
{
	// dual of edge
	template <typename Float = double, typename Flag = int>
	class DualSegment
	{
	private:
		Float        _source[2];
		Float        _target[2];
		Flag         _flag;
		DualSegment *_prev;
		DualSegment *_next;

	public:
		DualSegment()
			: _flag(Flag(-1)), _prev(NULL), _next(NULL)
		{
		}

		DualSegment(const Float *source, const Float *target, Flag flag = Flag(-1))
			: _flag(flag), _prev(NULL), _next(NULL)
		{
			_source[0] = source[0];
			_source[1] = source[1];
			_target[0] = target[0];
			_target[1] = target[1];
		}

		DualSegment(const DualSegment &rhs)
			: _flag(rhs._flag), _prev(rhs._prev), _next(rhs._next)
		{
			_source[0] = rhs._source[0];
			_source[1] = rhs._source[1];
			_target[0] = rhs._target[0];
			_target[1] = rhs._target[1];
		}

		DualSegment& operator=(const DualSegment &rhs)
		{
			if (this != &rhs)
			{
				_source[0] = rhs._source[0];
				_source[1] = rhs._source[1];
				_target[0] = rhs._target[0];
				_target[1] = rhs._target[1];

				_flag = rhs._flag;
				_prev = rhs._prev;
				_next = rhs._next;
			}

			return *this;
		}

		const Float* source() const
		{
			return _source;
		}

		void set_source(const Float *s)
		{
			_source[0] = s[0];
			_source[1] = s[1];
		}

		const Float* target() const
		{
			return _target;
		}

		void set_target(const Float *t)
		{
			_target[0] = t[0];
			_target[1] = t[1];
		}

		Flag flag() const
		{
			return _flag;
		}

		void set_flag(Flag v)
		{
			_flag = v;
		}

		DualSegment* prev_segment()
		{
			return _prev;
		}

		void set_prev_segment(DualSegment *p)
		{
			_prev = p;
		}

		DualSegment* next_segment()
		{
			return _next;
		}

		void set_next_segment(DualSegment *n)
		{
			_next = n;
		}

		void middle_point(Float *p) const
		{
			p[0] = (_source[0] + _target[0]) * Float(0.5);
			p[1] = (_source[1] + _target[1]) * Float(0.5);
		}

		Float squared_length() const
		{
			Float dx = _target[0] - _source[0];
			Float dy = _target[1] - _source[1];

			return (dx * dx + dy * dy);
		}

		Float length() const
		{
			return std::sqrt(squared_length());
		}
	};
}

#endif
