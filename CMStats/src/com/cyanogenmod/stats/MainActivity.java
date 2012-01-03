package com.cyanogenmod.stats;

import android.app.Activity;
import android.app.NotificationManager;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.net.Uri;
import android.os.Bundle;
import android.text.Html;
import android.text.method.LinkMovementMethod;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.TextView;
import android.widget.CompoundButton.OnCheckedChangeListener;

public class MainActivity extends Activity {
    private static final String PREF_NAME = "CMStats";

    private CheckBox mCheckbox;
    private Button mPreviewButton;
    private Button mSaveButton;
    private Button mStatsButton;
    private TextView mLink;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main);
        setTitle(R.string.main_title);
        
        mLink = (TextView) findViewById(R.id.main_link);
        mLink.setText(Html.fromHtml(getResources().getString(R.string.main_link)));
        mLink.setMovementMethod(LinkMovementMethod.getInstance());

        mCheckbox = (CheckBox) findViewById(R.id.main_optin);
        mCheckbox.setOnCheckedChangeListener(new OnCheckedChangeListener(){

            public void onCheckedChanged(CompoundButton buttonView,
                    boolean isChecked) {
                SharedPreferences settings = getSharedPreferences(PREF_NAME, 0);
                SharedPreferences.Editor editor = settings.edit();
                editor.putBoolean("optin", isChecked);
                editor.putBoolean("firstboot", false);
                editor.commit();
                startReportingService();
            }
        });

        mPreviewButton = (Button) findViewById(R.id.main_btn_preview);
        mPreviewButton.setOnClickListener(new OnClickListener(){

            public void onClick(View arg0) {
                Intent i = new Intent(MainActivity.this, PreviewActivity.class);
                startActivity(i);
            }

        });

        mSaveButton = (Button) findViewById(R.id.main_btn_save);
        mSaveButton.setOnClickListener(new OnClickListener(){
            public void onClick(View v) {
                finish();
            }
        });

        mStatsButton = (Button) findViewById(R.id.main_show_stats);
        mStatsButton.setOnClickListener(new OnClickListener(){
            public void onClick(View arg0) {
                Uri uri = Uri.parse("http://cyanogenmod.com/stats");
                startActivity(new Intent(Intent.ACTION_VIEW, uri));
            }
        });

        SharedPreferences settings = getSharedPreferences(PREF_NAME, 0);
        boolean optin = settings.getBoolean("optin", true);

        SharedPreferences.Editor editor = settings.edit();
        editor.putBoolean("firstboot", false);
        editor.commit();

        mCheckbox.setChecked(optin);

        NotificationManager nm = (NotificationManager) getSystemService(Context.NOTIFICATION_SERVICE);
        nm.cancel(1);
    }

    private void startReportingService(){
        ComponentName cmp = new ComponentName(getPackageName(), ReportingService.class.getName());
        startService(new Intent().setComponent(cmp));
    }
}
