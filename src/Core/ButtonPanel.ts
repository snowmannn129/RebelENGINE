import { HTMLPanelElement } from './HTMLPanelElement';
import { Button } from './UIControls';
import { Style } from './Style';

export class ButtonPanel extends HTMLPanelElement {
    private button: Button;
    private buttonElement: HTMLButtonElement;

    constructor(name: string, button: Button) {
        super(name);
        this.button = button;
        this.buttonElement = document.createElement('button');
        this.setupButton();
    }

    private setupButton(): void {
        // Set up button element
        this.buttonElement.textContent = this.button.label;
        this.buttonElement.addEventListener('click', () => this.button.click());
        
        // Apply initial style
        const style = this.button.getStyle()?.getComputedStyle();
        if (style) {
            Object.assign(this.buttonElement.style, {
                backgroundColor: style.backgroundColor,
                color: style.foregroundColor,
                border: `${style.borderWidth}px solid ${style.borderColor}`,
                borderRadius: `${style.borderRadius}px`,
                padding: `${style.padding}px`,
                width: '100%',
                cursor: 'pointer',
                outline: 'none',
                textAlign: 'left'
            });
        }

        // Add button element to panel
        const wrapper = document.createElement('div');
        wrapper.style.width = '100%';
        wrapper.style.height = '100%';
        wrapper.appendChild(this.buttonElement);
        this.setElement(wrapper);
    }

    public setStyle(style: Style): void {
        super.setStyle(style);
        this.button.setStyle(style);
        
        // Update button element style
        const computedStyle = style.getComputedStyle();
        Object.assign(this.buttonElement.style, {
            backgroundColor: computedStyle.backgroundColor,
            color: computedStyle.foregroundColor,
            border: `${computedStyle.borderWidth}px solid ${computedStyle.borderColor}`,
            borderRadius: `${computedStyle.borderRadius}px`,
            padding: `${computedStyle.padding}px`
        });
    }

    public refresh(): void {
        super.refresh();
        this.button.refresh();
    }
}
