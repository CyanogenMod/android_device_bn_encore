package com.cyanogenmod.stats;

import android.app.Activity;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

public class PreviewActivity extends Activity {
    private TextView mIdText;
    private TextView mDeviceText;
    private TextView mVersionText;
    private TextView mCountryText;
    private TextView mCarrierText;
    private Button mDoneButton;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.preview);
        
        mIdText = (TextView) findViewById(R.id.preview_id_value);
        mIdText.setText(Utilities.getUniqueID(getApplicationContext()));

        mDeviceText = (TextView) findViewById(R.id.preview_device_value);
        mDeviceText.setText(Utilities.getDevice());
        
        mVersionText = (TextView) findViewById(R.id.preview_version_value);
        mVersionText.setText(Utilities.getModVersion());
        
        mCountryText = (TextView) findViewById(R.id.preview_country_value);
        mCountryText.setText(Utilities.getCountryCode(getApplicationContext()));
        
        mCarrierText = (TextView) findViewById(R.id.preview_carrier_value);
        mCarrierText.setText(Utilities.getCarrier(getApplicationContext()));
        
        mDoneButton = (Button) findViewById(R.id.preview_done_btn);
        mDoneButton.setOnClickListener(new OnClickListener() {

            public void onClick(View v) {
                finish();
            }
            
        });
    }
}
