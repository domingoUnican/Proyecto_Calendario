from functools import wraps, partial
def debug(func=None, *, prefix = ''):
    if func is None:
        return partial(debug,prefix=prefix)
    @wraps(func)
    def wrapper(*args,**kwargs):
        DEBUG = open('log.txt','a')
        DEBUG.write(f'\n{func.__qualname__}, {args}, {kwargs}')
        DEBUG.close()
        return func(*args, **kwargs)
    return wrapper
