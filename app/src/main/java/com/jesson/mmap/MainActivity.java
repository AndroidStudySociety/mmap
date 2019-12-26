package com.jesson.mmap;

import androidx.appcompat.app.AppCompatActivity;

import android.app.Service;
import android.content.ComponentName;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Environment;
import android.os.IBinder;
import android.os.ParcelFileDescriptor;
import android.util.Log;
import android.view.View;
import android.widget.TextView;

import java.io.FileDescriptor;
import java.io.FileInputStream;

public class MainActivity extends AppCompatActivity {


    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        findViewById(R.id.sample_text).setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
//                 ShareMemUtils shareMemUtils = new ShareMemUtils();
//                 int fd = shareMemUtils.open(Environment.getExternalStorageDirectory().getAbsolutePath()+"/test.txt",0);
//                 Log.d("", "startMem fd:" + fd);
//                 int address = shareMemUtils.get(fd);
//                 Log.d("", "startMem address:" + address);
//                 byte[] buffer = {'I','I','t'};
//                shareMemUtils.write(fd, buffer, buffer.length, address);
                 mem();
            }
        });
        // Example of a call to a native method
        //在开一个app 来度数据
    }

    private void mem() {
        Intent intent = new Intent(MainActivity.this, MyService.class);
        bindService(intent, new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {

                byte[] content = new byte[10];
                IMyAidlInterface iMemoryAidlInterface
                        = IMyAidlInterface.Stub.asInterface(service);
                try {
                    ParcelFileDescriptor parcelFileDescriptor = iMemoryAidlInterface.getParcelFileDescriptor();
                    FileDescriptor descriptor = parcelFileDescriptor.getFileDescriptor();
                    FileInputStream fileInputStream = new FileInputStream(descriptor);
                    int result = fileInputStream.read(content);
                    Log.e("===",result+"");
                } catch (Exception e) {
                }}

            @Override
            public void onServiceDisconnected(ComponentName name) {

            }
        }, Service.BIND_AUTO_CREATE);
    }
}
