### 一, 事件等待与通知

#### 1, 等待的函数

##### ① event_add_internal

```
	if (base->current_event == ev && (ev->ev_events & EV_SIGNAL)
	    && !EVBASE_IN_THREAD(base)) 
	{
		++base->current_event_waiters;
		// 等待 通知工作  
		EVTHREAD_COND_WAIT(base->current_event_cond, base->th_base_lock);
	}
```

##### ② event_del_internal

```
	if (base->current_event == ev && !EVBASE_IN_THREAD(base)) 
	{
		++base->current_event_waiters; // wait
		// 等待 通知工作  
		EVTHREAD_COND_WAIT(base->current_event_cond, base->th_base_lock);
	}
```

##### ③ event_active_nolock

```
	if (base->current_event == ev && !EVBASE_IN_THREAD(base)) 
	{
		++base->current_event_waiters;
		// 等待 通知工作  
		EVTHREAD_COND_WAIT(base->current_event_cond, base->th_base_lock);
	}
```

#### 2. 广播的函数

#####	① event_process_active_single_queue

```
for (ev = TAILQ_FIRST(activeq); ev; ev = TAILQ_FIRST(activeq)) 
{
	if (ev->ev_events & EV_PERSIST)
	{
		event_queue_remove(base, ev, EVLIST_ACTIVE);
	}
	else
	{
		event_del_internal(ev);
	}
	if (!(ev->ev_flags & EVLIST_INTERNAL))
	{
		++count;
	}

	event_debug((
		 "event_process_active: event: %p, %s%scall %p",
		ev,
		ev->ev_res & EV_READ ? "EV_READ " : " ",
		ev->ev_res & EV_WRITE ? "EV_WRITE " : " ",
		ev->ev_callback));

#ifndef _EVENT_DISABLE_THREAD_SUPPORT
	base->current_event = ev;
	base->current_event_waiters = 0;
#endif

	switch (ev->ev_closure) 
	{
	case EV_CLOSURE_SIGNAL:
		event_signal_closure(base, ev);
		break;
	case EV_CLOSURE_PERSIST:
		event_persist_closure(base, ev);
		break;
	default:
	case EV_CLOSURE_NONE:
		EVBASE_RELEASE_LOCK(base, th_base_lock);
		(*ev->ev_callback)(
			(int)ev->ev_fd, ev->ev_res, ev->ev_arg);  //  调用业务的回调函数 
		break;
	}

	EVBASE_ACQUIRE_LOCK(base, th_base_lock);
#ifndef _EVENT_DISABLE_THREAD_SUPPORT
	base->current_event = NULL;
	if (base->current_event_waiters)
	{
		base->current_event_waiters = 0;
		// 通知 广播 main all thread wait事件 起来工作了 ^_^
		EVTHREAD_COND_BROADCAST(base->current_event_cond);
	}
#endif

	if (base->event_break)
	{
		return -1;
	}
}

```



### 二 ， 多线程

上面的等待是否是多线程每个线程独有event_base的都启动一个线程