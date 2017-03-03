#ifndef AUTO_FIELD_HPP_
#define AUTO_FIELD_HPP_

// TODO when merging this with Delegate and Task: figure out how to initialize without needing a copy constructor

#define auto_var(name, expr) decltype(expr) name = expr

#endif /* AUTO_FIELD_HPP_ */
