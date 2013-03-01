var actor = qtscript.actor();
actor.source = "wait_long_task_actor.js";
actor.send("Long task"
          , function(data) {
                print("returned from task, got", data);
            });
print("waiting for long task to be finished");
actor.wait();
print("this should appear after task is finished");
